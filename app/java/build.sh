
if [ "$1" = "clean" ]; then
	rm *.class
	rm ./com/letv/lelink/*.class
else

	cp ../../app/android/src/com/letv/lelink/LeCmd.java ./com/letv/lelink
	cp ./jni/Debug/liblelink.so ./com/letv/lelink
	javac ./com/letv/lelink/*.java
	javac *.java
fi

