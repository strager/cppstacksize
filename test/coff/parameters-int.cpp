void callee(
#if PARAM_COUNT >= 1
    int p0
#endif
#if PARAM_COUNT >= 2
    ,
    int p1
#endif
#if PARAM_COUNT >= 3
    ,
    int p2
#endif
#if PARAM_COUNT >= 4
    ,
    int p3
#endif
#if PARAM_COUNT >= 5
    ,
    int p4
#endif
#if PARAM_COUNT >= 6
    ,
    int p5
#endif
)
#if CALLEE
{
}
#else
    ;
#endif

#if CALLER
int caller() {
  callee(
#if PARAM_COUNT >= 1
      0xa00
#endif
#if PARAM_COUNT >= 2
      ,
      0xa01
#endif
#if PARAM_COUNT >= 3
      ,
      0xa02
#endif
#if PARAM_COUNT >= 4
      ,
      0xa03
#endif
#if PARAM_COUNT >= 5
      ,
      0xa04
#endif
#if PARAM_COUNT >= 6
      ,
      0xa05
#endif
  );

  // NOTE(strager): If PARAM_COUNT is small enough, then
  // MSVC optimizes 'callee(...)' into a tail call. To
  // avoid this, return some number instead of void.
  return 42;
}
#endif
