***MacOS와 같은 clang 환경에서는 makefile이 이루어지지 않으니 참고 바랍니다

# Project 1: MyLib

Below are examples of commands used in the `*.in` file. Your interactive program should work properly with each command as illustrated in the example.

| Command                                   | Description                                        |
| :---------------------------------------- | :------------------------------------------------- |
| `create list <LIST>`                      | Creates LIST.                                      |
| `create hashtable <HASH TABLE>`           | Creates HASH TABLE.                                |
| `create bitmap <BITMAP> <BIT CNT>`        | Creates BITMAP with the size of BIT CNT.           |
| `delete <LIST \| HASH TABLE \| BITMAP>`   | Deletes the given data structure.                  |
| `dumpdata <LIST \| HASH TABLE \| BITMAP>` | Prints the given data in data structure to STDOUT. |
| `quit`                                    | Terminates the interactive program.                |

### Simulation Example

Following is an example of running the interactive program.

```bash
$ ./testlib
create list list1
dumpdata list1
# Output: empty
create list list2
list_push_front list1 1
list_push_back list1 4
list_push_back list1 3
dumpdata list1
# Output: 1 4 3
list_max list1
# Output: 4
list_shuffle list1
dumpdata list1
# Output: 4 1 3
quit
```