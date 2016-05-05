#!/bin/bash
cd $(dirname "$0")
LINUX=$(ls | grep "^linux-3.14.[0-9]*$" | sort -n | tail -n1)
TEST=$1
TESTNAME=$(basename "$TEST")
rm -Rf mnt-tmp
mkdir mnt-tmp
mkdir mnt-tmp/bin
if ! test -z "$2"
then
        if ! test -d "$2"
	then 
		echo run_test_linux_spike.sh: Second argument must be a directory to merge into the root image
		echo Second argument is $2
		exit 2
	fi
        cp -a "$2"/* mnt-tmp/
fi
BLOCKS=${3:-65536}
INODES=${4:-1024}
rm -f run-test.sh
echo "#!/bin/ash" > run-test.sh
echo "if $TESTNAME; then echo \"COMPLETED SPIKE LINUX TEST\"; else echo \"FAILED SPIKE LINUX TEST\"; fi" >> run-test.sh
chmod +x run-test.sh
cp $TEST mnt-tmp/bin/$TESTNAME
echo "Generating disk image, this could take some time..."
./make_root.sh run-test.sh mnt-tmp/ $BLOCKS $INODES
set -m
LOG=log.test
echo | spike +disk=root.bin bbl $LINUX/vmlinux > $LOG 2>&1 &
JOBPID=$(jobs -l | grep spike | tr --squeeze " " | cut -d " " -f 2)
echo "Booting simulated Linux..."
tail -n +0 --follow=name $LOG 2>/dev/null | while read x; do x=$(echo "$x" | tr -d "\r\n"); if test "$x" == "COMPLETED SPIKE LINUX TEST"; then echo Successful completion; kill $JOBPID; rm $LOG; exit 1; fi; if test "$x" == "FAILED SPIKE LINUX TEST"; then echo Failed; cat $LOG; rm $LOG; kill $JOBPID; exit 2; fi; echo "$x"; done
RETVAL=$?
rm -Rf mnt-tmp run-test.sh root.bin
kill $JOBPID > /dev/null 2>&1
case $RETVAL in
	1)
		# Success
		exit 0
		;;
	2)
		# Failure
		exit 1
		;;
	0)
		# Still here.
		echo Spike broken???
		exit 2
		;;
esac
