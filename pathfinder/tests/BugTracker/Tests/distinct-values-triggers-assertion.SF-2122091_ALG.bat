@echo off

set q=distinct-values-triggers-assertion.SF-2122091_ALG.xq
set p=pf -A
@echo "%p% %q% >/dev/null"
@echo "%p% %q% >/dev/null" >&2
@%p% %q% > nul
