# OS

xv6 interrupt추가 및 스케쥴러 변경

# 1. Project01

[Project01](https://github.com/yudonlee/OS/blob/main/project01/README.md "Project01")
System call 추가 및 interrupt 추가  
| Project01-1 | Project01-2 |
| :---------: | :---------: |
| getppid() 구현 | interrupt 128 number 추가 |
| ![](https://github.com/yudonlee/OS/blob/main/image/project01_1_result.png) | ![](https://github.com/yudonlee/OS/blob/main/image/project01_2_result.png) |

# 2. xv6 스케쥴러 구현

[Project02](https://github.com/yudonlee/OS/blob/main/project02/README.md "Project02")

- FCFS scheduler
  | Result1 | Result2 | Result3 |
  | :-------------------------------------------------------------------: | :-------------------------------------------------------------------: | :-------------------------------------------------------------------: |
  | ![](https://github.com/yudonlee/OS/blob/main/image/fcfs_result_1.png) | ![](https://github.com/yudonlee/OS/blob/main/image/fcfs_result_2.png) | ![](https://github.com/yudonlee/OS/blob/main/image/fcfs_result_3.png) |
- Multilevel queue scheduler(Round Robin, FCFS)
  | Result1 | Result2 | Result3 | Result 4|
  | :-------------------------------------------------------------------: | :-------------------------------------------------------------------: | :-------------------------------------------------------------------: |:-------------------------------------------------------------------: |
  | ![](https://github.com/yudonlee/OS/blob/main/image/ml_result_1.png) | ![](https://github.com/yudonlee/OS/blob/main/image/ml_result_2.png) | ![](https://github.com/yudonlee/OS/blob/main/image/ml_result_3.png) | ![](https://github.com/yudonlee/OS/blob/main/image/ml_result_4.png) |

- MLFQ scheduler(Round robin, Priority scheduler)
  | Result1 | Result2 |
  | :-------------------------------------------------------------------: | :-------------------------------------------------------------------: |
  | ![](https://github.com/yudonlee/OS/blob/main/image/mlfq_result_1.png) | ![](https://github.com/yudonlee/OS/blob/main/image/mlfq_result_1.png) |
