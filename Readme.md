# System Programming (CSE4100)

서강대학교 시스템프로그래밍(CSE4100, 구 멀티코어프로그래밍) 과목의 프로젝트 제출본입니다.

본 저장소는 C언어 기반의 4가지 프로젝트로 구성되어 있습니다.

## Projects

### Project 1: MyLib
- **Description**: List, Hash Table, Bitmap 3개의 자료구조를 분석 및 구현하고, 이를 테스트하는 Interactive Program입니다.

#### 명령어 예시
아래는 `*.in` 파일에서 사용되는 명령어 예시입니다.

| 명령어                                    | 설명                              |
| :---------------------------------------- | :-------------------------------- |
| `create list <LIST>`                      | LIST를 생성                       |
| `create hashtable <HASH TABLE>`           | HASH TABLE을 생성                 |
| `create bitmap <BITMAP> <BIT CNT>`        | BITMAP을 BIT CNT 크기로 생성      |
| `delete <LIST \| HASH TABLE \| BITMAP>`   | 해당 자료구조 삭제                |
| `dumpdata <LIST \| HASH TABLE \| BITMAP>` | 자료구조의 데이터를 STDOUT에 출력 |
| `quit`                                    | 프로그램 종료                     |

#### 시뮬레이션 예시
아래는 인터랙티브 프로그램 실행 예시입니다.

```bash
$ ./testlib
create list list1
dumpdata list1
# 출력: empty
create list list2
list_push_front list1 1
list_push_back list1 4
list_push_back list1 3
dumpdata list1
# 출력: 1 4 3
list_max list1
# 출력: 4
list_shuffle list1
dumpdata list1
# 출력: 4 1 3
quit
```


### Project 2: MyShell

- **목표**: 리눅스/유닉스 환경에서 동작하는 간단한 커맨드라인 쉘을 직접 구현합니다.
- **주요 기능**
	- 사용자의 명령어 입력을 받아 실행
	- 백그라운드 실행 지원 (`&`)
	- 내장 명령어 처리(예: `quit` 등)
	- 여러 phase(1~3)로 점진적 기능 확장

- **실행 방법**
	```sh
	cd prj2/phase1
	make
	./myshell
	```

### Project 3

- **Concurrent Server** : Using Event-Based server model and Thread-Based server model

### Project 4: Malloc Lab
