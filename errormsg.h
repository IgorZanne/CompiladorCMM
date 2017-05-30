extern int EM_anyErrors;

void EM_newline(void);

extern int EM_tokPos;

void EM_error(int, char*,...);
void EM_impossible(char*,...);
void EM_reset(char* filename);
