unsigned int file_read(unsigned int id, void *buffer, unsigned int count);
unsigned int file_readall(unsigned int id, void *buffer, unsigned int count);
unsigned int file_write(unsigned int id, void *buffer, unsigned int count);
unsigned int file_writeall(unsigned int id, void *buffer, unsigned int count);
unsigned int file_seekread(unsigned int id, void *buffer, unsigned int count, unsigned int offset);
unsigned int file_seekreadall(unsigned int id, void *buffer, unsigned int count, unsigned int offset);
unsigned int file_seekwrite(unsigned int id, void *buffer, unsigned int count, unsigned int offset);
unsigned int file_seekwriteall(unsigned int id, void *buffer, unsigned int count, unsigned int offset);
