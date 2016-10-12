echo
echo '#ifdef __MACH__'
echo '#define prefix "_"'
echo '#else'
echo '#define prefix ""'
echo '#endif'
printf '#define asm_f(x) void Aundefined_##x () { __asm__("call " prefix "undefined_" #x "\\ntestl %%eax, %%eax\\nje endof_" #x "\\njmp *%%eax\\nendof_" #x ":\\n"); }'
echo
echo '#define undefi_name(x) undefined_ ##x'
printf '#define undefi(x) void* undefi_name(x) () { fprintf(stderr, "function #%%04d @ 0x%%08X (%%s)\\r\\n", x, functions[x].pointer, functions[x].name); return (void*)functions[x].pointer; }'
echo
echo

for i in `seq 0 8000`
do
echo "asm_f($i)"
done

for i in `seq 0 8000`
do
echo "undefi($i)"
done

echo 'void *pointers[] = {'
for i in `seq 0 8000`
do
echo "Aundefined_${i}+3, "
done
echo "NULL};"
echo
