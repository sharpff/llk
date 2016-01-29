import sys
# filename=raw_input('enter file name:') 
try:
    filename = sys.argv[1] 
    f=open(filename,'rb')
    f.seek(0,0)  
    index=0  

    while True:  
        temp=f.read(1)  
        if len(temp) == 0:  
            break  
        else:  
            print "0x%2s," % temp.encode('hex'),
            index = index + 1  
        if index == 16:
            index = 0  
            print   
    f.close()
except:
    print 'exec [input file]'