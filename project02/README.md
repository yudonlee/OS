# xv6 스케쥴러 구현

# 분류

1. FCFS scheduler
2. Multilevel queue scheduler(Round Robin, FCFS)
3. MLFQ scheduler(Round robin, Priority scheduler)

# 1. FCFS scheduler

## rule

1. 먼저 생성(fork())된 프로세스가 먼저 스케줄링 된다.
2. 기본적으로 스케쥴링된 프로세스는 종료되기 전까지 switch out되지 않으나, 실행시간이 200ticks 이상이고 sleeping상태로 전환되지 않으면 강제종료 한다.
3. sleeping상태로 전환된 프로세스가 발생하면, 그다음 프로세스가 스케쥴링 되며 다시 깨어나게 된다면, 다시 그 프로세스가 스케쥴링 된다.

## 디자인

- xv6의 proc.c::scheduler()함수에서 프로세스 전환을 담당한다. ptable엔 모든 프로세스가 저장되어 있고 해당 함수가 ptable을 확인하여 작동할 함수를 선택한다.
- 이때 RUNNABLE한 프로세스들 중 가장 오래전 만들어진 프로세스를 스케쥴링한다.
- 이때 원래 xv6는 round robin구조이기 때문에 1ticks마다 스케쥴러가 호출되는데, 우선순위 process가 계속 runnable한 상태라면 self-context-switch를 하게 된다.(이미 작동중인 프로세스가 RUNNABLE이고 우선순위가 가장 높은상태에서 스케쥴러를 다시 호출하더라도 현재 프로세스가 채택되기 때문이다.)
- 우선순위의 기준은 process 구조체 내에 creation_time이라는 변수를 추가하여 생성 시점을 할당했으나, pid를 통해 비교하는 방향으로 바꾸었다.
- 200ticks내에 초기화되기 위해선, 해당 시점을 기록하는 변수를 추가해야 한다.
- 추가된 변수에 프로세스가 스케쥴링 된 시점을 할당하고, 현재 시간과의 차이가 200ticks 내여야한다. 이때 round robin으로 기존에 구현된 구조로 인하여 스케쥴러는 1ticks마다 self-context-switch할 것인지 다음 프로세스를 사용할 것인지 결정한다.
- 그리하여 self-context-switch 도중 200ticks이상 차이가 날 때, 해당 프로세스의 kill변수가 true가 된다.
- trap.c의 trap함수에 의해 timer interrupt가 발생할 때 killed될 프로세스를 확인하여 종료시킨다.

## 구현(대표코드)

### 1. 프로세스가 첫 스케쥴링 된 시점을 기록할 변수 생성

```c
struct proc {
// in proc.h
	uint start_time;             // Process scheduling start time(for FCFS)
};
process가 allocate될 때, allocproc()에서 ptable을 탐색하여 UNUSED된 index를 찾아 새로운 프로세스를 할당할 때, start_time = 0으로 할당한다.(아직 스케쥴링 되기 전 이기 때문이다.)
```

### 2. ptable을 read하여 대상 프로세스 채택

```c
struct proc *minimum_p = 0;
// proc.c
		for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            // ...
			if(p->pid > -1){
				if(minimum_p != 0) {
					if(p->pid < minimum_p->pid || minimum_p->pid < 2){
						minimum_p = p;
					}
					if(minimum_p->state != RUNNABLE){
						minimum_p = p;
					}
				}
				else{
					minimum_p = p;
				}
			}
```

전체 ptable들 중 pid가 가장 작은 runnable process를 채택한다. minimum process를 찾기위해 전체 프로세스를 읽을 때, pid가 2보다 작은 shell이나 init process는 처음 스케쥴링 돼서 작동할 때를 제외하곤 호출되선 안된다.(재부팅 위험이 존재한다.)

### 3. minimum pid process scheduling

```c
// proc.c
// ...
if(p->start_time == 0)
				p->start_time = ticks;
```

(2)에서 채택한 프로세스를 스케쥴링 대상 프로세스로 할당한다. 이때 처음 실행한 프로세스라면 start_time을 현재 시간으로 할당해준다.

#### 4. 스케쥴링 된 프로세스가 200ticks 이상일 때 kill

```c
if(ticks - p->start_time > 200 && p->start_time != 0 && p->state == RUNNABLE && p->pid > 2){
			//p->start_time = 0;
			p->killed = 1;
```

context switch하고 난 뒤, 200ticks 이상인 프로세스는 kill해야한다.  
이때 exit()함수를 사용하지 않고 kill 변수를 true화 시켜 timer interrupt 발생 시 trap에서 자연스럽게 종료하도록 한다.  
이때 init프로세스는 모든 프로세스의 부모 프로세스이므로, 해당 프로세스가 종료될 시 좀비 프로세스가 만들어질 수 있기에 pid가 3이상인 경우에만 종료시킨다.

## 테스트 프로세스 p2_fcfs_text.c

p2_fcfs_test.c에 의해 만들어진 프로세스를 통해 결과가 출력된다. 해당 프로세스는 process가 yield, sleep, infinite process를 생성하여 우선순위대로 작동하는지 확인한다.

## 실행결과

|                                Result1                                |                                Result2                                |                                Result3                                |
| :-------------------------------------------------------------------: | :-------------------------------------------------------------------: | :-------------------------------------------------------------------: |
| ![](https://github.com/yudonlee/OS/blob/main/image/fcfs_result_1.png) | ![](https://github.com/yudonlee/OS/blob/main/image/fcfs_result_2.png) | ![](https://github.com/yudonlee/OS/blob/main/image/fcfs_result_3.png) |

Sleep일때를 제외하고는 전부 process id의 순서대로 나와야한다.  
왜냐하면 sleep과 yield가 없을때에는 당연히 FCFS 스케쥴링에 의해서 순서대로 작동하며, yield의 경우라고 해도 process를 양보하지만 실제적인 스케쥴링에서는 pid가 우선순위로 작동하기 떄문에 현재의 프로세스가 결국 self-context-switch 하게 된다.  
sleep인 경우 FCFS가 우선적이긴 하지만 RUNNABLE의 상태에 따라 결과값이 바뀔수 있다.

# 2. Multilevel queue scheduling

## Rule

1. 스케줄러는 2개의 큐로 이루어집니다. pid가 짝수일경우 Round Robin으로, 홀수일 경우 FCFS으로 스케쥴링 됩니다.
2. Round Robin 큐가 FCFS 큐보다 우선순위가 높습니다.
3. SLEEPING 상태에 있는 프로세스는 무시해야 하기에, RR큐에서 RUNNABLE한 프로세스가 없을 때 FCFS 큐를 확인한다.

## 디자인

1. 기존에 구현한 FCFS와 xv6에 구현된 Round robin 스케쥴러를 혼합한다.
2. ptable을 lookup하며, 우선적으로 짝수 pid를 가지는 프로세스를 스케쥴링하고, round robin이 사용되지 않을 경우 FCFS로 스케쥴링한다.

## 구현

1. 스케쥴링 대상 탐색(pid가 짝수일 때)

```c
int round_robin_used = 0;
   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state != RUNNABLE || p->pid % 2 == 1)
        continue;
    round_robin_used = 1; //if round_robin used
    }
```

짝수 pid를 가진 runnable 프로세스가 발견되면, 스케쥴링한다. 그리고 나서, Round Robin을 사용하였다는 의미로 변수 Round_robin_used를 1로 처리한다.

2. 짝수 pid에 RUNNABLE한 프로세스가 없을때

```c
if(round_robin_used == 0){
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state != RUNNABLE || p->pid % 2 == 0) continue;
        // ...FCFS와 동일
    }
```

round robin이 작동하지 않았을 때 FCFS 스케쥴링이 작동한다. 이때 fcfs_used 변수를 사용해줌으로써, FCFS 스케쥴링이 일어났을 때 200ticks 이상일 경우 kill할수 있는 조건을 만들어준다.

## 테스트 프로세스 p2_ml_text.c

p2_ml_test.c에 의해 만들어진 프로세스를 통해 결과가 출력된다.  
해당 프로세스는 process가 yield, sleep, infinite process를 생성하여 우선순위대로 작동하는지 확인한다.  
Test1에서는 단순히 fork된 프로세스들이 생성되고, Test2에서는 일부 yield가 존재하며, Test3에서는 sleep이 존재합니다.

## 실행결과

Test1에서는 짝수 프로세스가 먼저 RR로 실행되고, 홀수 프로세스가 작동한다.  
Test2에서는 process10과 12는 작동하지만 9와 11은 sleep되고 난 뒤, FCFS 스케쥴링에 의해 200ticks 이상 시간이 지났기 때문에 소멸한다.  
Test3에서는 우선적으로 짝수가 정리되고, sleep처리되면서 홀수 pid가 나오는과정이 반복된다. 결국 짝수 pid를 가진 프로세스가 먼저 끝나고, 그 후 홀수 pid를 가진 프로세스가 끝이 난다.
| Result1 | Result2 | Result3 | Result 4|
| :-------------------------------------------------------------------: | :-------------------------------------------------------------------: | :-------------------------------------------------------------------: |
:-------------------------------------------------------------------: |
| ![](https://github.com/yudonlee/OS/blob/main/image/ml_result_1.png) | ![](https://github.com/yudonlee/OS/blob/main/image/ml_result_2.png) | ![](https://github.com/yudonlee/OS/blob/main/image/ml_result_3.png) | ![](https://github.com/yudonlee/OS/blob/main/image/ml_result_4.png) |

# 2. MLFQ scheduling

## Rule

1. 2-level feedback queue (Disabling / enabling preemption)
2. L0와 L1 두 개의 큐로 이루어져 있고, L0의 우선순위가 더 높습니다.
3. 각 큐는 각각 다른 time quantum을 가집니다.
4. 처음 실행된 프로세스는 모두 가장 높은 레벨의 큐(L0)에 들어갑니다.
5. Starvation을 막기 위해 priority boosting이 구현되어야 합니다.

## 디자인

- Monopolized 된 process는 하나만 존재하기 때문에 이미 존재하는 Monopolized된 프로세스가 있다면, L0와 L1큐가 작동하지 않는다.
- Process 구조체 내부에 현재 프로세스가 속한 queue의 level을 나타내는 queue_level 변수와, L0큐에서 소모한 ticks를 나타내는 used_ticks_zero, L1큐에서 소모한 ticks를 나타내는 used_ticks_one을 구현한다.
- 그리고 소모한 시간을 통해 L0큐에서 L1큐로 이동시켜주는 것을 결정한다. 또한 Starvation을 막기위한 Priorty scheduling을 구현하기 위해서 구조체 내부에 priority변수또한 설정한다.
- FCFS 스케쥴링에서 사용된 start_time변수역시 priority boosting을 위해 사용되어야한다.
- L0에서의 사용시간을 나타내는 used_tick_zero는 queue_level이 1이 된다면 L0큐에서 모든 시간을 소비했기 때문에, L1큐에서 스케쥴링 될 때 함께 값을 0으로 바꾸어준다.  
  L0큐에서 스케쥴링 되었을 때 바로 0으로 해주지 않은 까닭은 time quantum을 전부 소모하지 않은 채 terminate된 process와, sleep된 process들의 경우 미리 변경한 queue_level을 0으로 다시 바꾸어주어야 한다.  
  이러한 상황을 증명하기 위해 사용되는 조건변수이기 때문에 바로 0으로 바꿔주지 않는 것이다.
- Monopolized된 process가 없을 때, Round Robin큐인 L0가 먼저 실행되고, 만약 이때 L0큐에서 작동될 때, queue_level을 올려준다.
- 만약 L0에서 작동중 프로세스가 zombie화 되었다면 L1에서 소비되지 않은 채 끝난 것이기 때문에, level을 다시 0으로 바꿔준다.

- 만약 zombie화 되지 않고 sleeping되었다고 해도, 주어진 time quantum을 전부 소비하지 못한채 sleeping된 상태이기 때문에 queue의 레벨을 다시 0으로 올려준다. 그외의 경우 주어진 time quantum을 소비하였음에도 RUNNABLE 상태라면, L1큐에 있게 된다.
- L0큐에 대기하는 프로세스가 없을때까지 기다리고, L1큐에서 실행한다.

- L0에서 RR을 통해 time quantum 4만큼을 소진한 프로세스들은 L1큐에서 8ticks로 작동해야 하고, 이들은 priority에 우선순위에 따라 스케쥴링이 되어야한다.  
  그떄 8ticks를 전부 소모한다면 priority를 -1해주어야 하는데, 이를 위해서는 process 구조체 내부에서 ticks의 사용량을 측정해야한다.  
  L0큐와 L1큐의 time quantum이 4와 8이기 때문에, timer interrupt를 4일떄마다 발생시켜주고, L0큐가 비어있다면 L1큐가 작동하게 된다.  
  이때 8ticks를 따로 하는 것이 아닌 4ticks를 두번하는 개념으로 생각하여 프로세스 구조체 내부에 queue_1_used라는 별도의 변수를 설정했다.  
  이것을 통해 만약 할당받은 8ticks를 모두 소진한다면 priority를 감소시켜준다. 이때 queue_1_used에는 8이 아닌 4ticks를 두번 사용할수 있기 때문에 2를 넣었다.

- 할당받은 8ticks를 줄여주기 위해서, 프로세스들은 스케쥴링 되었을 때 남은 스케쥴링횟수가 1이라면(timer interrupt를 4로 설정했기 때문에 1번의 스케쥴링 후에는 priority가 감소되어야 한다.) 프로세스가 끝나고의 유무와 상관없이 일단 priority를 감소시켜주어야한다.  
  왜냐하면 L1큐에서는 time quantum을 전부 소비하지 않은 채 작업이 끝나서 process가 terminate됐을 때라고 할지라도 L0큐와 같이 queue의 level을 조절하는 과정이 필요하지 않기 때문이다.(L1에서 종료된 사실만 중요하기 때문)

## 결과분석

#실행결과
Priorty boosting의 과정을 통해 L0로 들어온다 해도, 다수의 for-loop를 도는 테스트 프로그램 특성상 L0큐에서 terminate되는 프로세스는 전체 프로세스 중의 일부일수밖에 없다.  
Priority를 계속해서 새롭게 주는 Focused priorty에서는, 각자 프로세스들의 priority가 다르기 때문에 상대적으로 L1큐에서 높은 우선순위를 받은 프로세스들이 동작을 계속 진행하기 때문에, priority를 따로 설정하지 않는 without priority manipulation에 비해 L0큐에 속한 개수가 적게 된다.  
yield에서 계속해서 다음 프로세스를 스케쥴링 하는 과정에서 다음 프로세스에게 프로세스를 양보하면서 프로세스 스케쥴링이 발생하는데, 계속해서 다음 프로세스에게 양보하고 이러한 양상으로 인해 대체로 4개의 프로세스 값은 거의 동일한 값이 나온다.  
또한 yield case의 경우, 모든 프로세스가 yield가 실행되기 전 L1에서 variable이 6000씩 오른 뒤, yield()로의 출력이 잡히게 되면서 time quantum을 전부 소비하지 않은 상태에서 계속해서 스케쥴링이 진행되기 때문에 L0에서 계속 머무르게 되어 값이 균일하게 나오게 된다.  
monopolize에서는, 다른 process들은 monopolized되지 않았기 때문에 L0값과 L1값이 스케쥴링 양상으로 값이 나오게 되고, monopolized된 process 23는 L0큐에 머무르면서 독점적으로 프로세스를 사용하며 종료된다.

|                                Result1                                |                                Result2                                |
| :-------------------------------------------------------------------: | :-------------------------------------------------------------------: |
| ![](https://github.com/yudonlee/OS/blob/main/image/mlfq_result_1.png) | ![](https://github.com/yudonlee/OS/blob/main/image/mlfq_result_1.png) |
