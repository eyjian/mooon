// Writed by yijian on 2018/1/29
//
// Linux批量上传到远程机器工具
// 相比C++版本，借助go的特性，不依赖libc和libc++等库，编译出的二进制应用相对广泛
//
// 注意：
// 如果目标文件已存在，则会上传失败！
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
// 依赖的scp包：
// 从https://github.com/tmc/scp下载，
// 放到目录$GOROOT/src/github.com/tmc，
// 解压后目录名需为scp，如为scp-master则需要重命名scp
//
// scp又依赖go-shellquote，
// 从https://github.com/kballard/go-shellquote下载，
// 放到目录$GOROOT/src/github.com/kballard，
// 解压后目录名需为go-shellquote，如为go-shellquote-master则需要重命名为go-shellquote
//
// 参数-h：指定远程机器的IP列表，如果是多个IP，则IP间以逗号分隔，可用环境变量H替代
// 参数-P：指定远程机器的SSH端口号，可用环境变量PORT替代
// 参数-u：连接远程机器的用户名，可用环境变量U替代
// 参数-p：连接远程机器的密码，可用环境变量P替代
// 参数-s：需要上传的单个或多个文件，多个文件间以逗号分隔
// 参数-d：上传到哪个目录
//
// 编译方法：
// go build -o mooon_upload mooon_upload.go

// main函数的package名只能为main，否则运行报：
// cannot run non-main package
package main

import (
    "golang.org/x/crypto/ssh"
    "github.com/tmc/scp"
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
    g_hosts = flag.String("h", "", " Connect to the remote machines on the given hosts separated by comma, can be replaced by environment variable 'H'")
    g_port = flag.Int("P", 22, "Specifies the port to connect to on the remote machines, can be replaced by environment variable 'PORT'")
    g_user = flag.String("u", "", "Specifies the user to log in as on the remote machines, can be replaced by environment variable 'U'")
    g_password = flag.String("p", "", "The password to use when connecting to the remote machines, can be replaced by environment variable 'P'")
    g_sources = flag.String("s", "", "Local source files uploaded, separated by comma")
    g_destination = flag.String("d", "", "Remote destination directory")
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

    // sources
    if *g_sources == "" {
        fmt.Printf("Parameter[\033[1;33m-s\033[m] not set\n\n")
        usage()
        os.Exit(1)
    }

    // destination
    if *g_destination == "" {
        fmt.Printf("Parameter[\033[1;33m-d\033[m] not set\n\n")
        usage()
        os.Exit(1)
    }

    host_array := strings.Split(hosts, ",")
    for _, host:=range host_array {
        ip_port := host + ":" + fmt.Sprintf("%d", port)
        Upload2Remote(ip_port, user, password)
        fmt.Printf("\n")
    }
}

func Upload2Remote(ip_port string, user string, password string) {
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

    filepath_array := strings.Split(*g_sources, ",")
    for _, filepath:=range filepath_array {
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
                                
                file, err := os.Open(filepath)
                if err != nil {
                    fmt.Printf("[\033[0;32;31mERROR\033[m] Open %s error: %s\n", filepath, err.Error())
                } else {
                    defer file.Close()
                    st, err := file.Stat()

                    if err != nil {
                        fmt.Printf("[\033[0;32;31mERROR\033[m] Stat %s error: %s\n", filepath, err.Error())
                    } else {
                        err = scp.Copy(st.Size(), st.Mode().Perm(), filepath, file, *g_destination, session)
                        if err != nil {
                            fmt.Printf("[\033[0;32;31mERROR\033[m] Upload %s to %s%s error: %s\n", filepath, ip_port, *g_destination, err.Error())
                        } else {
                            fmt.Printf("[\033[1;33mOK\033[m] upload %s to %s%s\n", filepath, ip_port, *g_destination);
                        }
                    }
                }
            }
        }
    }
}

func usage() {
    flag.Usage()
    fmt.Printf("\n")
    fmt.Printf("Format:\nmooon_upload -h=host1,host2,... -P=port -u=user -p=password -s=source_filepath1,source_filepath2,... -d=destination_directory\n")
    fmt.Printf("\n")
    fmt.Printf("Example:\nmooon_upload -h=192.168.31.32 -P=22 -u=root -p='root@2018' -s=mssh,mupload -d=/usr/local/bin\n")
    fmt.Printf("\n")
}
