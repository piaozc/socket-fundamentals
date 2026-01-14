#这是socket网络编程学习情况，不知道会不会有人看呢，大概是不会的

##week1
基本实现了tcp服务端的逻辑，采用多线程方法实现并发
测试使用nc测试，轻松通过50条连接请求并且可以收发数据

##week2
实现select模型的并法，区别于week1的直接人工填写协议族，这次使用更现代的getaddrinfo（）/*别忘了释放socket，我就忘了*/
同样使用nc测试，这次是100条，感觉比多线程处理起来快不少

##week3
学习epoll，理解epoll底层原理，epoll instance包含一个interest list（红黑树）和一个ready list（就绪队列），本质上只处理活跃节点，区别于select和poll的全部遍历
上传了两个版本，LT和ET
LT使用nc测试，在700多条连接的时候服务器崩溃
ET使用pv测试，成功通过了10000条连接，cpu占用率从27%上升到55%，测试效果非常好