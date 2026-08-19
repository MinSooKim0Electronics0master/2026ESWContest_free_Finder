# 깃허브 최소 숙지 매뉴얼 (토요일까지, 민수용)

> 그림이 포함된 인쇄용 문서는 [github-study.docx](github-study.docx)입니다.
> 이 파일은 같은 내용의 텍스트판입니다.

## 1. 목표

이 문서는 토요일 모임 전까지 깃허브(GitHub)의 기본 사용법을 익히기 위한
최소 매뉴얼입니다. 분량은 하루 30분 × 3일입니다. 목표는 하나입니다 —
**토요일에 아무것도 보지 않고, 연습 저장소에서 「브랜치 만들기 → 수정 →
커밋 → Push → PR → 병합」을 혼자 할 수 있으면 준비가 끝난 것입니다.**

실제 프로젝트의 작업 규칙은 [github-guide.md](github-guide.md)에 있으며,
이 문서는 그 전에 보는 학습용입니다.

## 2. 준비물 (10분)

1. github.com 계정이 필요합니다. 없으면 미리 가입합니다.
2. **GitHub Desktop**을 설치합니다: https://desktop.github.com
3. 설치 후 로그인합니다: `File` → `Options` → `Accounts` → GitHub.com 옆
   `Sign in`

## 3. 기본 개념 4가지

| 용어 | 뜻 |
|---|---|
| 저장소 (repository) | 프로젝트 폴더와 지금까지의 모든 변경 기록입니다 |
| 커밋 (commit) | 변경 묶음 하나를 메시지와 함께 저장한 것입니다 |
| 브랜치 (branch) | main을 건드리지 않고 작업하기 위해 복사한 작업 줄기입니다 |
| PR (pull request) | 내 브랜치를 main에 합쳐 달라는 요청입니다. 합치기 전에 리뷰를 받습니다 |

## 4. 1일 차 (수요일, 30분) — 설치와 연습 저장소

실수해도 되는 개인 연습 저장소를 만듭니다. 이번 주 실습은 모두 이
저장소에서 합니다.

1. 웹(github.com) 오른쪽 위 **+** 버튼을 누르고 **New repository**를
   선택합니다.
2. 이름은 `practice-repo`, 공개 범위는 **Private**를 선택하고,
   **Add a README file**에 체크한 뒤 **Create repository** 버튼을 누릅니다.
3. 만들어진 저장소 페이지에서 초록 **Code** 버튼 →
   **Open with GitHub Desktop** → **Clone** 순서로 누르면 저장소가 내 PC로
   복사됩니다.
4. GitHub Desktop 상단의 세 구역 위치를 눈에 익힙니다 —
   **Current repository**(어느 저장소인지) / **Current branch**(어느
   브랜치인지, 가장 중요) / **Fetch origin**(서버 최신 상태 확인).

1일 차는 여기까지입니다. 화면 위치만 익혀도 충분합니다.

## 5. 2일 차 (목요일, 30분) — 수정 → 커밋 → Push

1. Desktop의 `Repository` 메뉴 → **Show in Explorer**를 누르면 저장소
   폴더가 열립니다. README.md를 메모장 등으로 열어 한 줄을 추가하고
   저장합니다.
2. Desktop 왼쪽 **Changes** 탭에 수정한 파일이 보이고, 오른쪽에 변경
   내용이 표시됩니다. 빨간 줄은 지운 내용, 초록 줄은 새로 넣은 내용입니다.
3. 왼쪽 아래 **Summary** 칸에 커밋 메시지를 쓰고 **Commit to main** 버튼을
   누릅니다. 메시지는 `영역: 한 줄 요약` 형식입니다.
   예: `docs: 자기소개 한 줄 추가`
4. 오른쪽 위 버튼이 **Push origin**으로 바뀝니다. 누른 뒤 웹에서 저장소를
   새로 고침해 방금 수정이 올라갔는지 확인합니다.
5. 새 파일(memo.md 등)을 만들어 같은 과정을 한 번 더 반복합니다.

> 참고 — 오늘은 연습이므로 main에 직접 커밋했습니다. 실제 프로젝트에서는
> main 직접 커밋이 금지이며, 3일 차에 배우는 방식(브랜치 → PR)이 정식
> 절차입니다.

## 6. 3일 차 (금요일, 30분) — 브랜치 → PR → 병합

먼저 루틴입니다. 작업하려고 앉으면 항상 다음 4단계를 순서대로 합니다.
이것은 외워야 합니다.

> **① Fetch origin → ② Current branch가 main인지 확인 → ③ Pull origin →
> ④ New Branch**

오늘 실습:

1. 가운데 **Current Branch**를 누르고 **New Branch** 버튼 → 이름
   `test/first-pr` 입력 → **Create Branch**를 누릅니다.
2. 2일 차처럼 파일을 수정하고 커밋합니다. 커밋 버튼이
   `Commit to test/first-pr`로 바뀐 것을 확인합니다.
3. 오른쪽 위 **Publish branch**를 누릅니다. 처음 올리는 브랜치는 Push 대신
   이 이름으로 표시됩니다.
4. **Preview Pull Request** 버튼으로 변경 내용을 확인한 뒤
   **Create Pull Request**를 누르면 브라우저가 열립니다. 제목을 확인하고
   **Create pull request** 버튼을 누릅니다.
5. PR 화면의 세 탭(**Conversation / Commits / Files changed**)을 차례로
   눌러 봅니다. 리뷰 코멘트는 Files changed 탭의 줄에 달립니다.
6. **Merge pull request** → **Confirm merge**를 누르고, **Delete branch**로
   서버의 브랜치를 정리합니다. 병합된 브랜치는 지워도 기록이 남습니다.
7. Desktop으로 돌아와 **Current Branch**에서 main을 선택하고
   **Pull origin**을 누릅니다. 방금 병합한 내용이 내 PC의 main에 내려온
   것을 확인합니다. `Branch` 메뉴 → **Delete...**로 로컬 브랜치도
   정리합니다.

여기까지 하면 실전에서 쓰는 흐름을 전부 한 바퀴 돈 것입니다. 시간이 남으면
한 바퀴 더 반복합니다.

## 7. 실수했을 때 — 버튼 3개

| 상황 | 누르는 것 | 효과 |
|---|---|---|
| 커밋 전 | Changes 탭에서 파일 우클릭 → **Discard Changes…** | 수정 자체를 되돌립니다. 복구 불가 — 파일명 꼭 확인 |
| 커밋 직후 (Push 전) | 왼쪽 아래 **Undo** | 커밋만 풀리고 수정 내용은 남습니다. 고쳐서 다시 커밋 |
| Push까지 한 뒤 | History 탭에서 커밋 우클릭 → **Revert Changes in Commit** | 되돌리는 새 커밋이 생깁니다. 옛 커밋을 지우지 않습니다 |

세 경우 모두 판단이 애매하면 **아무것도 누르지 말고** 화면을 그대로 둔 채
이령에게 보여 주면 됩니다. 질문이 수습보다 빠릅니다.

## 8. 토요일 자가 점검

다음 다섯 가지를 아무것도 보지 않고 할 수 있으면 준비 완료입니다.

- [ ] 1. 앉으면 하는 루틴 4단계를 말할 수 있다 (Fetch → main 확인 → Pull
  → New Branch)
- [ ] 2. 커밋 메시지 형식 `영역: 한 줄 요약`을 알고 예를 하나 들 수 있다
- [ ] 3. 연습 저장소에서 브랜치 → 수정 → 커밋 → PR → 병합을 5분 안에 혼자
  할 수 있다
- [ ] 4. 실수 수습 버튼 3개(Discard Changes / Undo / Revert Changes in
  Commit)가 어디 있는지 안다
- [ ] 5. 금지 4항을 말할 수 있다 — main 직접 push 금지 / 브랜치 확인 없이
  시작 금지 / 100 MB 파일 금지 / 한글 파일명 지양

3번에서 막히면 3일 차 실습을 한 번 더 반복하면 됩니다. 막히는 화면이
나오면 잘못 누르기 전에 그대로 두고 사진을 찍어 보내면 됩니다.
