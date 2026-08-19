# 토요일까지 — 깃허브 최소 숙지 매뉴얼 (민수용)

목표는 딱 하나다. 토요일에 아무것도 안 보고, 연습 저장소에서
**"브랜치 만들기 → 수정 → 커밋 → Push → PR → 병합"** 을 혼자 할 수
있으면 준비 끝이다.

하루 30분씩 3일이면 된다. 서두르지 말고 순서대로. 외울 것은 마지막
"토요일 자가 점검" 5개뿐이다.

> 이 문서는 토요일 전 학습용이고,
> [github-guide.md](github-guide.md)는 실제 작업 중 계속 펴 놓는 문서다.

---

## 준비물 (10분이면 끝)

- github.com 계정 (이미 있으면 통과)
- **GitHub Desktop** 설치: https://desktop.github.com
  - 설치 후 로그인: `File` → `Options` → `Accounts` → GitHub.com 옆
    `Sign in`

## 개념 4개 — 이것만 알면 된다

| 용어 | 뜻 |
|---|---|
| 저장소 (repository) | 프로젝트 폴더 + 지금까지의 모든 변경 기록 |
| 커밋 (commit) | "저장" 스냅샷 한 장. 메시지를 붙여서 남긴다 |
| 브랜치 (branch) | 원본(main)을 건드리지 않고 작업하는 복사 줄기 |
| PR (pull request) | "내 브랜치를 main에 합쳐 주세요" 요청. 합치기 전에 리뷰를 받는 관문 |

비유: main은 **제출용 완성본**, 브랜치는 **연습장**, 커밋은 연습장의
**중간 저장**, PR은 **검사 맡기**다.

---

## 수요일 (오늘, 30분) — 설치와 연습 저장소

실수해도 되는 내 연습 저장소를 하나 만든다. 이번 주 실습은 전부 여기서
한다.

1. 웹(github.com) 오른쪽 위 **`+`** → **`New repository`**
   - Repository name: `practice-repo`
   - **Private** 선택
   - **`Add a README file`** 체크
   - **`Create repository`** 버튼
2. 만들어진 저장소 페이지에서 초록 **`< > Code`** 버튼 →
   **`Open with GitHub Desktop`** → 뜨는 창에서 **`Clone`**
   (내 PC로 복사된다)
3. Desktop 화면 위쪽 세 칸의 위치를 눈에 익힌다 — 앞으로 매일 보는 곳:
   - 왼쪽: **Current repository** (지금 어느 저장소인가)
   - 가운데: **Current branch** (지금 어느 브랜치인가) ← 제일 중요
   - 오른쪽: **Fetch origin** (서버 최신 상태 확인)

오늘은 여기까지. 화면 구경만 해도 충분하다.

## 목요일 (30분) — 수정 → 커밋 → Push

1. Desktop에서 `Repository` 메뉴 → **`Show in Explorer`** → 열린 폴더의
   `README.md`를 메모장(아무 에디터)으로 열어 한 줄 추가하고 저장
2. Desktop으로 돌아오면 왼쪽 **Changes** 탭에 파일이 떠 있고, 오른쪽에
   빨간 줄(지운 것)·초록 줄(넣은 것)이 보인다 — 이 화면이 "커밋할 내용
   미리 보기"다
3. 왼쪽 아래 **Summary** 칸에 메시지를 쓰고 **`Commit to main`** 버튼
   - 메시지 형식은 우리 규칙대로: **`영역: 한 줄 요약`**
   - 예: `docs: 자기소개 한 줄 추가`
4. 오른쪽 위가 **`Push origin`** 으로 바뀐다 → 누른다 → 웹에서 저장소를
   새로고침해 방금 줄이 올라갔는지 확인
5. 한 번 더 반복한다. 이번에는 새 파일(`memo.md` 등)을 만들어서

> 참고: 오늘은 연습이라 main에 직접 커밋했다. **실제 프로젝트에서는
> 금지**다 — 내일 배우는 방식(브랜치 → PR)이 실전 방식이다.

## 금요일 (30분) — 실전 루틴: 브랜치 → PR → 병합

먼저 루틴부터. 앞으로 작업하려고 앉으면 무조건 이 4단계다:

> **① Fetch origin → ② Current branch가 main인지 확인 → ③ Pull origin
> → ④ New Branch**

오늘 실습:

1. 가운데 **Current Branch** 클릭 → **`New Branch`** 버튼 → 이름
   `test/first-pr` → **`Create Branch`**
2. 어제처럼 파일을 수정하고 커밋한다 (커밋 대상이
   `Commit to test/first-pr`로 바뀐 것을 확인)
3. 오른쪽 위 **`Publish branch`** — 처음 올리는 브랜치는 Push 대신 이
   이름으로 뜬다
4. **`Preview Pull Request`** 버튼 → 변경 내용을 훑어보고 →
   **`Create Pull Request`** → 브라우저가 열린다 → 제목 확인 →
   **`Create pull request`** 버튼
5. 웹의 PR 화면에서 탭 세 개를 구경한다:
   **Conversation**(대화) / **Commits**(커밋 목록) /
   **Files changed**(바뀐 줄 — 리뷰 코멘트가 달리는 곳)
6. **`Merge pull request`** → **`Confirm merge`** → **`Delete branch`**
   (병합된 브랜치는 지워도 기록이 남는다)
7. Desktop으로 돌아와 **Current Branch** → `main` 선택 →
   **`Pull origin`** → 방금 병합한 내용이 내 PC의 main에 내려온 것을
   확인. `Branch` 메뉴 → **`Delete...`** 로 로컬의 `test/first-pr`도 정리

여기까지 했으면 실전에서 쓸 흐름을 전부 한 바퀴 돈 것이다. 시간이 남으면
한 바퀴 더.

---

## 토요일 자가 점검 — 안 보고 답하기

1. 앉으면 하는 루틴 4단계는? (Fetch → main 확인 → Pull → New Branch)
2. 커밋 메시지 형식은? 예시 하나 말하기 (`영역: 한 줄 요약`)
3. 연습 저장소에서 "브랜치 → 수정 → 커밋 → PR → 병합"을 5분 안에 혼자
4. 실수 수습 버튼 3개의 이름과 상황 — 위치만 알면 된다:
   - 커밋 **전**: 파일 우클릭 → **Discard Changes**
   - 커밋 **직후**(Push 전): 왼쪽 아래 **Undo**
   - **Push 후**: History 탭 → 커밋 우클릭 → **Revert Changes in Commit**
5. 금지 4항 말하기: main 직접 push 금지 / 브랜치 확인 없이 시작 금지 /
   100 MB 파일 금지 / 한글 파일명 지양

5개 다 되면 준비 완료. 3번에서 막히면 금요일 실습만 한 번 더 돌면 된다.
막히는 게 있으면 그 화면 그대로 두고 이령에게 보여 줄 것 — 잘못 누르는
것보다 물어보는 게 백 배 낫다.
