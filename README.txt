세마포를 이용하여 Lock과 Variable을 만들어 구성된 Mesa Style Monitor로 Reader & Writer Problem을 구현한다. 

초기값이 1인 세마포를 만들어 P()는 Acquire()로 V()는 Release()로 활용한다. 

세마포의 waiting queue가 비어있는지를 간접적으로 알기위해 별도의 카운터를 관리한다. 이 카운터는 method 함수에 모두 공유되어야 하는데 여기서는 화일 변수 (fileVar)로 구현하기로 한다. 


Reader, Writer가 협력하여 관리하는 4개의 카운터(AR, AW, WR, WW)는 4개의 화일에 기록 하기로 한다. 즉 화일의 초기값은 0이며 값이 변할 때마다 변한 값을 추가하여 기록한다. 이때 기록된 시간 및 기록한 프로세스의 ID(pid, getpud()로 취득)도 함께 기록한다. 이 4개의 카운터값을 상황에 맞게 변경하는 것을 잘 동기화하는 것이 관건이다. 실행이 끝난 후 4개의 화일을 통합하여 시간 순으로 정렬하면 실행동안 발생한 상황을 분석할 수 있다. 


-reader, writer 구현
 ex) reader 3 8  //3 초 후에 8초동안  읽음.



