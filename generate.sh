echo '#define undefi_name(x) undefined_ ##x'
echo '#define undefi(x) void undefi_name(x) () { fprintf(stderr, "undefined %d\n", x); void (*foo)(void) = functions[x].pointer; foo(); }'

for i in `seq 0 8000`
do
echo "undefi($i)"
done

echo 'void *pointers[] = {'
for i in `seq 0 8000`
do
echo "undefi_name($i), "
done
echo "NULL};"

