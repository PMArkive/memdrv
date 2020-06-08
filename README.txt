memory r/w driver
Hooks disk.sys ioctl by placing a call instruction to unused function and then rewriting the major function pointer.