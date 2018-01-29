// Writed by yijian on 2018/1/29
//
// Linux批量远程执行命令工具
// 相比C++版本，借助go的特性，不依赖libc和libc++等库
//
// 依赖的crypto包：
// 从https://github.com/golang/crypto下载，
// 放到目录/usr/lib/golang/src/golang.org/x下
//
// 参数-h：指定远程机器的IP列表，如果是多个IP，则IP间以逗号分隔，可用环境变量H替代
// 参数-P：指定远程机器的SSH端口号，可用环境变量PORT替代
// 参数-u：连接远程机器的用户名，可用环境变量U替代
// 参数-p：连接远程机器的密码，可用环境变量P替代
// 参数-c：需要远程执行的命令，建议使用单引号括起来，如：-c='ls -l'

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
    g_help = flag.Bool("H", false, "Help information")
    g_hosts = flag.String("h", "", "IP list, separated by commas")
    g_port = flag.Int("P", 22, "Remote SSH port")
    g_user = flag.String("u", "", "User")
    g_password = flag.String("p", "", "Password")
    g_command = flag.String("c", "", "Command")
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
            fmt.Printf("Parameter[-h] not set\n")
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
            fmt.Printf("Parameter[-P]: invaid port\n")
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
            fmt.Printf("Parameter[-u] not set\n")
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
            fmt.Printf("Parameter[-p] not set\n")
            usage()
            os.Exit(1)
        }
    }
    
    // command
    if (*g_command == "") {
        fmt.Printf("Parameter[-c] not set\n")
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
    fmt.Printf("\033[1;33m")
    fmt.Printf("[%s]\n", ip_port)
    fmt.Printf("\033[m")

    client, err := ssh.Dial("tcp", ip_port,  &ssh.ClientConfig{
        User: user,
        Auth: []ssh.AuthMethod{
            ssh.Password(password),
        },
        HostKeyCallback: func(hostname string, remote net.Addr, key ssh.PublicKey) error {
            return nil
        },
    })
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
    fmt.Printf("Format:\nmssh -h=host1,host2,... -P=port -u=user -p=password -c=command\n")
    fmt.Printf("Example:\nmssh -h=192.168.31.32 -P=22 -u=root -p='root@2018' -c='whoami'\n")
}

