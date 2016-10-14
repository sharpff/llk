
if [ "$1" = "clean" ]; then
	rm *.class
	rm ./com/letv/lelink/*.class
else
	pushd ./jni > /dev/null 2>&1
	make
	popd > /dev/null 2>&1

	cp ../../app/android/src/com/letv/lelink/LeCmd.java ./com/letv/lelink
	cp ./jni/Debug/liblelink.so .
	javac ./com/letv/lelink/*.java
	javac *.java
fi

