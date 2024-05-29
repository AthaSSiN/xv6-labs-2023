mkdir a
echo hello > a/b
mkdir c
echo hello > c/b
echo hello > b
mkdir a/aa
echo hello > a/aa/b
find . b | xargs grep hello
