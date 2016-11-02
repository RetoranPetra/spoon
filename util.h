#undef strlcat
size_t strlcat(char *, const char *, size_t);
#undef strlcpy
size_t strlcpy(char *, const char *, size_t);

#define LEN(x) (sizeof (x) / sizeof *(x))

#ifdef DEBUG
#define DPRINTF_S(x) printf(#x "=%s\n", x)
#define DPRINTF_U(x) printf(#x "=%u\n", x)
#define DPRINTF_D(x) printf(#x "=%d\n", x)
#else
#define DPRINTF_S(x)
#define DPRINTF_U(x)
#define DPRINTF_D(x)
#endif
