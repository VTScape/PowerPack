#include "fcontrol.c"

int main(){
	powerpack_fp_init("mydatafile.txt");
	powerpack_fp_session_start("foo");
	printf("Hello world");
	powerpack_fp_session_end("foo");
	powerpack_fp_end("mydatafile.txt");
}
