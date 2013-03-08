void logit(const char *file, const int line, const char *str, ...);
#define LOG(...) logit(__FILE__, __LINE__, __VA_ARGS__)
