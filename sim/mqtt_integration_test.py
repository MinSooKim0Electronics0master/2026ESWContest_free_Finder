#!/usr/bin/env python3
"""Wokwi Finder-C 한 개와 수행하는 MQTT 통합 시험입니다.

PC가 A 역할로 FinderPacket 10건을 100ms 간격으로 발행하고, 동시에 D
역할로 C의 재송신을 구독합니다. 외부 패키지 없이 MQTT 3.1.1의 필요한
기능만 Python 표준 라이브러리로 구현합니다.
"""

import argparse
import random
import socket
import struct
import sys
import time
import uuid


if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")


DEFAULT_BROKER = "broker.hivemq.com"
DEFAULT_PORT = 1883
DEFAULT_TOPIC = "eswcontest/finder-2026-hybrid-8f4a2c/lora"

NODE_A = 1
NODE_C = 3
FACILITY_AED = 1
STATUS_OK = 0
TTL_INITIAL = 4
PACKET_STRUCT = struct.Struct("<IBBBBB")
RADIO_PAYLOAD_SIZE = 1 + PACKET_STRUCT.size
QUEUE_CAPACITY = 16


def encode_remaining_length(value):
    encoded = bytearray()
    while True:
        digit = value % 128
        value //= 128
        if value:
            digit |= 0x80
        encoded.append(digit)
        if not value:
            return bytes(encoded)


def encode_utf8(value):
    raw = value.encode("utf-8")
    return struct.pack("!H", len(raw)) + raw


def receive_exact(sock, size):
    chunks = bytearray()
    while len(chunks) < size:
        chunk = sock.recv(size - len(chunks))
        if not chunk:
            raise ConnectionError("MQTT 브로커가 연결을 종료했습니다")
        chunks.extend(chunk)
    return bytes(chunks)


def receive_packet(sock):
    first_byte = receive_exact(sock, 1)[0]
    multiplier = 1
    remaining_length = 0
    for _ in range(4):
        digit = receive_exact(sock, 1)[0]
        remaining_length += (digit & 0x7F) * multiplier
        if not digit & 0x80:
            return first_byte, receive_exact(sock, remaining_length)
        multiplier *= 128
    raise ValueError("잘못된 MQTT Remaining Length입니다")


def decode_publish(first_byte, body):
    if len(body) < 2:
        raise ValueError("MQTT PUBLISH 본문이 너무 짧습니다")
    topic_length = struct.unpack("!H", body[:2])[0]
    topic_end = 2 + topic_length
    if len(body) < topic_end:
        raise ValueError("MQTT PUBLISH 토픽 길이가 잘못되었습니다")
    topic = body[2:topic_end].decode("utf-8")
    payload_start = topic_end
    qos = (first_byte >> 1) & 0x03
    if qos:
        payload_start += 2
    return topic, body[payload_start:]


class MqttClient:
    def __init__(self, broker, port, topic):
        self.broker = broker
        self.port = port
        self.topic = topic
        self.sock = None
        self.packet_id = 1

    def send_packet(self, first_byte, body=b""):
        packet = bytes([first_byte]) + encode_remaining_length(len(body)) + body
        self.sock.sendall(packet)

    def connect(self):
        self.sock = socket.create_connection((self.broker, self.port), timeout=10)
        self.sock.settimeout(1.0)
        client_id = f"finder-pc-{uuid.uuid4().hex[:12]}"
        variable_header = (
            encode_utf8("MQTT")
            + bytes([4, 0x02])
            + struct.pack("!H", 30)
        )
        self.send_packet(0x10, variable_header + encode_utf8(client_id))
        first_byte, body = receive_packet(self.sock)
        if first_byte >> 4 != 2 or len(body) != 2 or body[1] != 0:
            raise ConnectionError(f"MQTT 연결 거부: {body!r}")

    def subscribe(self):
        packet_id = self.packet_id
        self.packet_id += 1
        body = struct.pack("!H", packet_id) + encode_utf8(self.topic) + b"\x00"
        self.send_packet(0x82, body)
        while True:
            first_byte, response = receive_packet(self.sock)
            if first_byte >> 4 != 9:
                continue
            if len(response) < 3:
                raise ConnectionError("잘못된 MQTT SUBACK입니다")
            response_id = struct.unpack("!H", response[:2])[0]
            if response_id == packet_id and response[2] != 0x80:
                return
            raise ConnectionError("MQTT 구독이 거부되었습니다")

    def publish(self, payload):
        self.send_packet(0x30, encode_utf8(self.topic) + payload)

    def close(self):
        if self.sock is None:
            return
        try:
            self.send_packet(0xE0)
        except OSError:
            pass
        self.sock.close()
        self.sock = None


def make_radio_payload(sequence):
    msg_id = (NODE_A << 24) | (sequence & 0x00FFFFFF)
    packet = PACKET_STRUCT.pack(
        msg_id,
        NODE_A,
        FACILITY_AED,
        STATUS_OK,
        TTL_INITIAL,
        NODE_A,
    )
    return bytes([NODE_A]) + packet


def unpack_relay(payload):
    if len(payload) != RADIO_PAYLOAD_SIZE:
        return None
    tx_id = payload[0]
    fields = PACKET_STRUCT.unpack(payload[1:])
    return (tx_id, *fields)


def run_test(args):
    if not 1 <= args.count <= QUEUE_CAPACITY:
        raise ValueError(f"--count는 1~{QUEUE_CAPACITY}여야 합니다")
    if args.interval_ms < 1:
        raise ValueError("--interval-ms는 1 이상이어야 합니다")

    max_start = 0x00FFFFFF - args.count
    start_sequence = args.start_sequence
    if start_sequence is None:
        start_sequence = random.SystemRandom().randint(1, max_start)
    if not 1 <= start_sequence <= max_start:
        raise ValueError("--start-sequence 범위가 잘못되었습니다")

    expected_sequences = list(
        range(start_sequence, start_sequence + args.count)
    )
    expected_set = set(expected_sequences)
    received_counts = {sequence: 0 for sequence in expected_sequences}
    field_errors = []

    client = MqttClient(args.broker, args.port, args.topic)
    try:
        print(f"[PC] MQTT 연결: {args.broker}:{args.port}")
        client.connect()
        client.subscribe()
        print(f"[PC] 구독 완료: {args.topic}")
        if not args.no_prompt:
            input(
                "Wokwi Finder-C에 '브로커 연결 완료'가 보이면 "
                "Enter를 누르십시오: "
            )

        print(
            f"[A] 패킷 {args.count}건 발신: "
            f"{args.interval_ms}ms 간격, 시작 sequence={start_sequence}"
        )
        for sequence in expected_sequences:
            client.publish(make_radio_payload(sequence))
            time.sleep(args.interval_ms / 1000.0)

        duplicate_sequence = expected_sequences[len(expected_sequences) // 2]
        client.publish(make_radio_payload(duplicate_sequence))
        print(f"[A] 중복 확인용 sequence={duplicate_sequence} 재발신")

        deadline = time.monotonic() + args.timeout
        all_received_at = None
        while time.monotonic() < deadline:
            if all_received_at is not None and time.monotonic() - all_received_at >= 1.0:
                break
            try:
                first_byte, body = receive_packet(client.sock)
            except socket.timeout:
                continue
            if first_byte >> 4 != 3:
                continue
            topic, payload = decode_publish(first_byte, body)
            if topic != args.topic:
                continue
            relay = unpack_relay(payload)
            if relay is None:
                continue
            (
                tx_id,
                msg_id,
                src_id,
                facility_type,
                status,
                ttl,
                last_hop_id,
            ) = relay
            sequence = msg_id & 0x00FFFFFF
            if tx_id != NODE_C or sequence not in expected_set:
                continue

            received_counts[sequence] += 1
            if (
                msg_id >> 24 != NODE_A
                or src_id != NODE_A
                or facility_type != FACILITY_AED
                or status != STATUS_OK
                or ttl != TTL_INITIAL - 1
                or last_hop_id != NODE_C
            ):
                field_errors.append(
                    f"sequence={sequence}: tx={tx_id}, src={src_id}, "
                    f"facility={facility_type}, status={status}, "
                    f"ttl={ttl}, lastHop={last_hop_id}"
                )
            if all(received_counts.values()) and all_received_at is None:
                all_received_at = time.monotonic()

        missing = [seq for seq, count in received_counts.items() if count == 0]
        duplicated = [seq for seq, count in received_counts.items() if count > 1]

        print(f"[D] 수신 결과: {args.count - len(missing)}/{args.count}")
        if missing:
            print(f"실패: 누락 sequence={missing}")
        if duplicated:
            print(f"실패: 중복 재송신 sequence={duplicated}")
        if field_errors:
            print("실패: 패킷 필드 불일치")
            for error in field_errors:
                print(f"  - {error}")
        if missing or duplicated or field_errors:
            return 1

        print("통과: 연속 패킷 전부 수신")
        print("통과: TTL=3, lastHopId=C 및 패킷 필드 일치")
        print("통과: 동일 msgId 중복 재송신 없음")
        return 0
    finally:
        client.close()


def parse_args():
    parser = argparse.ArgumentParser(
        description="Wokwi Finder-C 한 개와 수행하는 MQTT 통합 시험"
    )
    parser.add_argument("--broker", default=DEFAULT_BROKER)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument("--topic", default=DEFAULT_TOPIC)
    parser.add_argument("--count", type=int, default=10)
    parser.add_argument("--interval-ms", type=int, default=100)
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--start-sequence", type=int)
    parser.add_argument(
        "--no-prompt",
        action="store_true",
        help="Wokwi 준비 확인 Enter 입력을 생략합니다",
    )
    return parser.parse_args()


def main():
    try:
        return run_test(parse_args())
    except (ConnectionError, OSError, ValueError) as error:
        print(f"시험 실행 실패: {error}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
