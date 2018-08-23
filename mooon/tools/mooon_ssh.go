// Writed by yijian on 2018/1/29
//
// Linux批量远程执行命令工具
// 相比C++版本，借助go的特性，不依赖libc和libc++等库，编译出的二进制应用相对广泛
//
// 依赖的crypto包：
// 从https://github.com/golang/crypto下载，
// 放到目录$GOPATH/src/golang.org/x或$GOROOT/src/golang.org/x下
// 需要先创建好目录$GOROOT/src/golang.org/x，然后在此目录下解压crypto包
// 如果下载的包名为crypto-master.zip，则解压后的目录名为crypto-master，需要重命名为crypto
//
// 示例：
// 1）安装go
// cd /usr/local
// tar xzf go1.10.3.linux-386.tar.gz
// 2）mkdir -p go/golang.org/x
// 3）cd go/golang.org/x
// 4）unzip crypto-master.zip
// 5）mv crypto-master crypto
//
// 命令行执行“go help gopath”可了解gopath，或执行“go env”查看当前的设置
//
// 参数-h：指定远程机器的IP列表，如果是多个IP，则IP间以逗号分隔，可用环境变量H替代
// 参数-P：指定远程机器的SSH端口号，可用环境变量PORT替代
// 参数-u：连接远程机器的用户名，可用环境变量U替代
// 参数-p：连接远程机器的密码，可用环境变量P替代
// 参数-c：需要远程执行的命令，建议使用单引号括起来，如：-c='ls -l'
//
// 编译方法：
// go build -o mooon_ssh mooon_ssh.go

// main函数的package名只能为main，否则运行报：
// cannot run non-main package
package main

import (
    "golang.org/x/crypto/ssh"
    //flag "github.com/ogier/pflag"
    "flag"
    "fmt"
    "net"
    "os"
    "strconv"
    "strings"
)

var (
    g_help = flag.Bool("H", false, "Display a help message and exit")
    g_hosts = flag.String("h", "", "Connect to the remote machines on the given hosts separated by comma, can be replaced by environment variable 'H'")
    g_port = flag.Int("P", 22, "Specifies the port to connect to on the remote machines, can be replaced by environment variable 'PORT'")
    g_user = flag.String("u", "", "Specifies the user to log in as on the remote machines, can be replaced by environment variable 'U'")
    g_password = flag.String("p", "", "The password to use when connecting to the remote machines, can be replaced by environment variable 'P'")
    g_command = flag.String("c", "", "The command is executed on the remote machines")
)

func main() {
    var hosts, user, password string
    var port int
    flag.Parse()
    
    // help
    if *g_help {        
        usage()
        os.Exit(1)
    }
    
    // hosts
    if (*g_hosts != "") {
        hosts = *g_hosts
    } else {
        s := os.Getenv("H")
        if s != "" {
            hosts = s
        } else {
            fmt.Printf("Parameter[\033[1;33m-h\033[m] not set\n\n")
            usage()
            os.Exit(1)
        }
    }

    // port
    s := os.Getenv("PORT")
    if s == "" {
        port = *g_port
    } else {
        port_, err := strconv.Atoi(s)
        if err != nil {
            fmt.Printf("Parameter[\033[1;33m-P\033[m]: invaid port\n\n")
            usage()
            os.Exit(1)
        } else {
            port = port_
        }
    }

    // user
    if (*g_user != "") {
        user = *g_user
    } else {
        s := os.Getenv("U")
        if s != "" {
            user = s
        } else {
            fmt.Printf("Parameter[\033[1;33m-u\033[m] not set\n\n")
            usage()
            os.Exit(1)
        }
    }
    
    // password
    if (*g_password != "") {
        password = *g_password
    } else {
        s := os.Getenv("P")
        if s != "" {
            password = s
        } else {
            fmt.Printf("Parameter[\033[1;33m-p\033[m] not set\n\n")
            usage()
            os.Exit(1)
        }
    }
    
    // command
    if (*g_command == "") {
        fmt.Printf("Parameter[\033[1;33m-c\033[m] not set\n\n")
        usage()
        os.Exit(1)
    }
    
    host_array := strings.Split(hosts, ",")
    for _, host:=range host_array {
        ip_port := host + ":" + fmt.Sprintf("%d", port)
        RemoteExecute(ip_port, user, password)
    }
}

func RemoteExecute(ip_port string, user string, password string) {
    authMethods := []ssh.AuthMethod{}

    fmt.Printf("\033[1;33m")
    fmt.Printf("[%s]\n", ip_port)
    fmt.Printf("\033[m")
    
    keyboardInteractiveChallenge := func(
        user,
        instruction string,
        questions []string,
        echos []bool,
    ) (answers []string, err error) {
        if len(questions) == 0 {
            return []string{}, nil
        }
        return []string{*g_password}, nil
    }

    authMethods = append(authMethods, ssh.KeyboardInteractive(keyboardInteractiveChallenge))
    authMethods = append(authMethods, ssh.Password(*g_password))
    sshConfig := &ssh.ClientConfig{
        User: *g_user,
        Auth: authMethods,
        HostKeyCallback: func(hostname string, remote net.Addr, key ssh.PublicKey) error {
            return nil
        },
    }

    client, err := ssh.Dial("tcp", ip_port,  sshConfig)
    if err != nil {
        fmt.Printf("\033[0;32;31m");
        fmt.Printf("%s\n", err)
        fmt.Printf("\033[m")
    } else {
        defer client.Close()
        session, err := client.NewSession()
        if err != nil {
            fmt.Printf("\033[0;32;31m");
            fmt.Printf("%s\n", err)
            fmt.Printf("\033[m")
        } else {
            defer session.Close()
            reply, err := session.Output(*g_command)
            if err != nil {
                if err.Error() == "Process exited with status 255" {
                    fmt.Printf("[\033[0;32;31mERROR\033[m] command `%s` not exists\n", *g_command)
                } else if err.Error() == "Process exited with status 1" {
                    fmt.Printf("[\033[0;32;31mERROR\033[m] command `%s` return 1\n", *g_command)
                } else {
                    fmt.Printf("[\033[0;32;31mERROR\033[m] %s\n", err)
                }
            } else {
                fmt.Printf("%s\n", reply)
                fmt.Printf("[\033[1;33mOK\033[m][%s]\n\n", ip_port)
            }
        }
    }
}

func usage() {
    flag.Usage()
    fmt.Printf("\n")
    fmt.Printf("Format:\nmooon_ssh -h=host1,host2,... -P=port -u=user -p=password -c=command\n")
    fmt.Printf("\n")
    fmt.Printf("Example:\nmooon_ssh -h=192.168.31.32 -P=22 -u=root -p='root@2018' -c='whoami'\n")
    fmt.Printf("\n")
}
