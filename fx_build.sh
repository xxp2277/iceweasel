#! /usr/bin/bash
MYOBJ_DIR=
FIND_FILE=".mozconfig"
if [ ! -f "$FIND_FILE" ]; then
  echo $FIND_FILE not exist!
  exit 1;
fi
FIND_STR="ac_add_options --target=i686-pc-mingw32"
if [ "$OS" != "Windows_NT" ]; then
  PATH=$PATH:~/.cargo/bin
  MYOBJ_DIR=obju-linux64
  PYTHON_SCRIPT=_virtualenvs/init/bin
  MAKE=make
else
  if [ `grep -c "^$FIND_STR" $FIND_FILE` -ne '0' ];then
    [[ -n $MY_OBJ ]] && MYOBJ_DIR=$MY_OBJ || MYOBJ_DIR=obju32-release
  else
    [[ -n $MY_OBJ ]] && MYOBJ_DIR=$MY_OBJ || MYOBJ_DIR=obju64-release
  fi
  PYTHON_SCRIPT=_virtualenvs/init_py3/Scripts
  MAKE=mozmake
fi

rm -f ./configure
rm -f ./configure.old
autoconf-2.13
rm -rf ../$MYOBJ_DIR
mkdir ../$MYOBJ_DIR && cd ../$MYOBJ_DIR
../iceweasel/configure --enable-profile-generate=cross
$MAKE -j4
if [ "$?" != "0" ]; then
  echo First compilation failed. > error.log
  exit 1;
fi

$MAKE package
$PYTHON_SCRIPT/pip install -U selenium
$PYTHON_SCRIPT/python ../iceweasel/build/pgo/profileserver2.py

ls *.profraw >/dev/null 2>&1
if [ "$?" != "0" ]; then
  echo profileserver.py failed. >> error.log
  exit 1;
fi

$MAKE maybe_clobber_profiledbuild
../iceweasel/configure --enable-profile-use=cross --enable-lto=cross
$MAKE -j4 2>&1 | tee -i ../pgo_build.log
if [ "$?" != "0" ]; then
  echo Second compilation failed. >> error.log
  exit 1;
fi
$MAKE package
echo Compile completed!
