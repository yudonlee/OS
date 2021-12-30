#include "types.h"
#include "stat.h"
#include "user.h"
//이 세가지는 반드시 호출해주어야 하며 순서또한 반드시 지켜야한다. 
int main(){
	printf(1, "My pid is %d\n", getpid());
	printf(1, "My ppid is %d\n", getppid());	
	exit();
}
