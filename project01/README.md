# Project01

xv6에 getppid() System call 및 interrupt 128 추가

# Project 01-1, 01-2 실행결과

| Project01-1 | Project01-2 |
| :---------: | :---------: |
| getppid() 구현 | interrupt 128 number 추가 |
| ![](https://github.com/yudonlee/OS/blob/main/image/project01_1_result.png) | ![](https://github.com/yudonlee/OS/blob/main/image/project01_2_result.png) |


## 실행결과 분석

- Project01-1  
  현재 process id와 부모 process id가 출력되며, ppid와 pid는 보통 1차이가 나는 경우가 많으나 부모프로세스와 자식프로세스 사이에 다른 프로세스가 실행되는 경우에 따라서 값이 차이가 날수 있다.
- Project01-2  
  Interrupt128이 발생하였을떄는 trapno가 128로 되어 else if문에서 process가 종료되기 때문에, switch loo에서 default로 jump하지 않는다.

# 1. Project01-1

**getppid()**

Interrupt T_SYSCALL로 system call이 발생하면서 interrupt handler가 실행되고 interrupt handler가 eax번호에 해당하는 system call을 호출하게 된다.  
이때 eax register 에 저장된 번호는 syscall.c 안에 syscall(void)함수에서 실행되고 defined 된 번호에 해당하는 system call인 getppid가 실행이 된다.

## 디자인

1. 새로운 시스템 콜인 getppid() 선언 및 정의, system call 번호 정의
2. SYSCALL MACRO로 system call 번호를 eax에 저장(호출 statement로 사용된다)
3. wrapper function을 통해 전처리 작업(sys_getppid)
4. my_proc()을 통해 현재 cpu가 갖고있는 proc structure의 포인터를 활용한다.  
   해당 구조체가 갖고 있는 parent값을 가져온다.

## 코드 구현(대표 코드)

1. sysproc.c

```c
//sysproc.c
// 이미 구현된 부분
int
sys_getpid(void)
{
  return myproc()->pid;
}
// 위의 wrapper function을 참고하여 만든 부분
int
sys_getppid(void)
{
	return myproc()->parent->pid;
}
```

커널에 존재하는 다양한 함수들을 다른 함수에서 호출할 수 있도록 함수 원형을 선언한다.  
그리하여, 새롭게 추가된 getppid또한 사용될 수 있도록 extern으로 선언한다.

2. syscall.c

```c
// ...
static int (*syscalls[])(void) = {
    [SYS_getppid] sys_getppid,
};
```

xv6에서 system call을 인식하기 위해서 syscalls[]라는 배열의 index를 조절하여 접근할 수 있다. 함수 포인터형 배열이기 때문에, index를 통해 접근한 값들은 함수의 주소가 되고, 이값을 가지고 함수 호출이 가능하다.

3. xv6에서 사용할 프로세스 구현

```c
// projcet01_1.c
#include "types.h"
#include "stat.h"
#include "user.h"
//이 세가지는 반드시 호출해주어야 하며 순서또한 반드시 지켜야한다.
int main(){
	printf(1, "My pid is %d\n", getpid());
	printf(1, "My ppid is %d\n", getppid());
	exit();
}

```

현재 프로세스의 id를 가져오기 위해 getpid system call사용하고, 그다음 부모 프로세스의 id를 가져오기 위해 새롭게 구현한 getppid()를 호출한다.

# 2. Project01-2

**xv6에 interrupt 128 추가**  
256개의 Interrupt descriptor가 xv6에 존재할 때, 이중 64개의 interrupt 만이 유저에서 커널로 entrypoint를 switching하는데 사용된다.  
그리하여 우리는 interrupt number 128을 만든다.

## 디자인

1. project01_2.c에서 assembly language를 통해 Interrupt 128 발생 시킨다.
2. macro 정의를 통해 128을 가지는 T_USERSYSCALL 정의한다.
3. SETGATE macro를 통해 모든 cpu에 공유되는 interrupt descriptor table에 시스템 콜 번호를 set한다.
4. trap method를 통해서 유저가 정의한 interrupt가 발생될 때 print문을 출력할 수 있도록 한다.

## 구현

1. traps.h

```c
// traps.h
#define T_USERSYSCALL 128
```

traps.h의 헤더파일 내부에서 user syscall을 macro로 지정한다.

2. traps.c

### tvinit()

```c
void tvinit(void){
   // ...
   for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
   SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
   SETGATE(idt[T_USERSYSCALL], 1, SEG_KCODE<<3, vectors[T_USERSYSCALL], DPL_USER);
}
```

interrupt descriptor table중 T_USERSYSCALL에 해당하는 Index에 interrupt를 set해주어, SYSCALL 번호가 64외에 13이 되는 현상을 방지한다.

### trap()

```c
void trap(struct trapframe *tf){
   else if (tf->trapno == T_USERSYSCALL) {
         cprintf("user interrupt %d called!\n", tf->trapno);
         exit();
      }
}
```

trap()부분에 trapno에 따라서 if문으로 jump하고 swtich문으로 가는 flow가 존재하는데 이때 else if문으로 trapno가 T_USERSYSCALL과 같을 때 process를 종료하는 것을 추가한다.

3. project01_2.c xv6에서 호출될 프로세스 구현

```c
#include "types.h"
#include "stat.h"
#include "user.h"
//이 세가지는 반드시 호출해주어야 하며 순서또한 반드시 지켜야한다.
int main(){
	asm("int $128");
	return 0;
}
```

inline assembly를 사용하여 interrupt를 호출하도록 한다.

# Trouble Shooting

## 1. trap13 issue

![](https://github.com/yudonlee/OS/blob/main/image/project01_2_trouble_1.png)  
SETGATE의 중요성을 모른채 무작정 trap.c에서 trap()에 else if statement를 추가하고, traps.h에서 T_USERSYSCALL을 128로 define macro를 이용해서 정의한 뒤, xv6를 실행하였다.  
하지만 결과적으로 계속 trapno가 13이 나온다는 사실을 발견했고, 과제 명세에서도 trapno가 13일때를 정답이 인정되지 않는다는 글을 보고 과제를 해결할 단서를 찾았다.  
그리하여 cscope를 통해 T_SYSCALL을 사용하는 모든 부분을 체크하여, interrupt set을 하는 방식을 배우게 되었다.
