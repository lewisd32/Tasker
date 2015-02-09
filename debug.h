
//#define SER_TRACE

#ifdef SER_Trace
#define serTrace(arg) Serial.print(arg)
#define serTraceln(arg) Serial.println(arg)
#else
#define serTrace(arg)
#define serTraceln(arg)
#endif

//#define SER_DEBUG

#ifdef SER_DEBUG
#define serDebug(arg) Serial.print(arg)
#define serDebugln(arg) Serial.println(arg)
#else
#define serDebug(arg)
#define serDebugln(arg)
#endif

//#define SER_INFO

#ifdef SER_INFO
#define serInfo(arg) Serial.print(arg)
#define serInfoln(arg) Serial.println(arg)
#else
#define serInfo(arg)
#define serInfoln(arg)
#endif

#define SER_WARN

#ifdef SER_WARN
#define serWarn(arg) Serial.print(arg)
#define serWarnln(arg) Serial.println(arg)
#else
#define serWarn(arg)
#define serWarnln(arg)
#endif

// vim: set tabstop=2 shiftwidth=2 softtabstop=2 smarttab expandtab:
