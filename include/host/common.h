#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __GNUC__
#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif
#else
#ifndef likely
#define likely(x)	(x)
#endif
#ifndef unlikely
#define unlikely(x)	(x)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

void _LogOut(const char* sLog);
void _ErrorOut(const char* sError);

#ifdef __cplusplus
}
#endif

#endif