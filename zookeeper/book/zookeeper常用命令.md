Zookeeper的常用命令有：
[定位 find ./ -name 'zkServer.sh']
启动Zookeeper服务：./zkServer.sh start
查看Zookeeper服务状态：./zkServer.sh status
停止Zookeeper服务：./zkServer.sh stop
重启Zookeeper服务：./zkServer.sh restart
Zookeeper客户端常用命令有：

连接Zookeeper服务端：zkCli.sh -server ip:port
断开连接：quit
查看命令帮助：help
显示指定目录下节点：ls 目录
创建节点：create /节点path value
获取节点值：get /节点path
设置节点值：set /节点path value
删除单个节点：delete /节点path
您可以根据具体需求使用这些命令来操作Zookeeper。