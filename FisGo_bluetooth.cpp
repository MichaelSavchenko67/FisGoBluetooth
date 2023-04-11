#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <boost/regex.hpp>
#include <syslog.h>
#include <mutex>
#include "FisGo_bluetooth.h"

#define LOG_NAME "libFisGo_bluetooth"



std::mutex mutex_bt;


static std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        syslog(LOG_ERR,"%s:popen() failed!",LOG_NAME);
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

#define EXTFE(fn, st) {int ret = fn;if (!WIFEXITED(ret)) {syslog(LOG_ERR,st,LOG_NAME); return false;}}
static bool CheckBluetoothdRun()
{
    static const std::string CHECK_BLUETOOTHD_RUN = "pidof bluetoothd";
    std::string dout = exec(CHECK_BLUETOOTHD_RUN.c_str());
    return !dout.empty();

}

static bool CheckHciRun()
{
    static const std::string CHECK_BLUETOOTH_RUN = "hciconfig hci0 | grep 'UP'";
    return !exec(CHECK_BLUETOOTH_RUN.c_str()).empty();
}


bool Fisgo_Bluetooth::CheckRun()
{
    mutex_bt.lock();
    bool status = (CheckBluetoothdRun() && CheckHciRun());
    mutex_bt.unlock();
    return status;
}


static const std::string REGEX_IP = "[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}";
#define SIZE_IP 17

static bool CheckIp(std::string ip)
{
    if(ip.length() != SIZE_IP)
    {
       // syslog(LOG_ERR,"%s:Wrong size ip");
        syslog(LOG_ERR,"%s: Wrong size ip",LOG_NAME);
        return false;
    }
    const boost::regex txt_regex(REGEX_IP);
    try
    {
        const boost::regex txt_regex(REGEX_IP);
        return boost::regex_match(ip, txt_regex);
    }
    catch (boost::exception &e)
    {
        syslog(LOG_ERR,"%s:Error boost::regex txt_regex",LOG_NAME);
        return false;
    }
    //return boost::regex_match(ip, txt_regex);
 // return true;
}



static bool SerchData(std::string data_in, std::string& data_out,std::string data_regex)
{

    // Simple regular expression matching
    boost::smatch base_match;
    boost::regex txt_regex(data_regex);
    try
    {
        if(boost::regex_search(data_in,base_match, txt_regex))
        {
            boost::ssub_match base_sub_match = base_match[0];
            data_out = base_sub_match.str();
            return true;
        }
    }
    catch (boost::exception &e)
    {
        syslog(LOG_ERR,"%s:Error boost::regex regex_search",LOG_NAME);
        return false;
    }


    return false;

}



bool Fisgo_Bluetooth::On()
{
    mutex_bt.lock();
    if(!CheckHciRun() && !CheckBluetoothdRun())
    {
        static const std::string BLUETOOTH_UP = "hciconfig hci0 up piscan";
        static const std::string BLUETOOTH_RUN = "bluetoothd &";
        EXTFE(system(BLUETOOTH_UP.c_str()), "%s:Error BLUETOOTH_UP");
        usleep(500);
        if (!CheckHciRun())
        {
            syslog(LOG_ERR,"%s:not run hci",LOG_NAME);
            mutex_bt.unlock();
            return false;
        }
        EXTFE(system(BLUETOOTH_RUN.c_str()), "%s:Error BLUETOOTH_RUN");
        usleep(500);
        if(!CheckBluetoothdRun())
        {
            for(int i = 0;i<10;i++)
            {
                usleep(500);
                if(CheckBluetoothdRun()) {
                    mutex_bt.unlock();
                    return true;
                }
            }
            syslog(LOG_ERR,"%s:not run bluetoothd",LOG_NAME);
            EXTFE(system("hciconfig hci0 down"),"%s:Error hciconfig hci0 down");
            mutex_bt.unlock();
            return false;
        }
        mutex_bt.unlock();
        return true;
    }
    mutex_bt.unlock();
    return false;
}

static std::vector<std::string> splitString(std::string divisibleStr, char delimiter)
{
    std::vector<std::string> rv;

    if ( divisibleStr.empty() )
        return rv;

    std::string tmpString;
    std::istringstream streamStr(divisibleStr);

    while ( getline(streamStr, tmpString, delimiter) )
    {
        rv.push_back( tmpString );
    }

    return rv;
}


bool Fisgo_Bluetooth::Scan(std::vector<DeviceInfo>& sc)
{
    mutex_bt.lock();
    if(CheckHciRun())
    {
        DeviceInfo tmp_sc;
        try
        {
            for (const auto &line : splitString(exec("hcitool scan"), '\n'))
            {
                boost::smatch match;
                boost::regex tmpl("^((\\s{0,})(?<BT_MAC>(([[:xdigit:]]{2}[:.-]){5}[[:xdigit:]]{2}))(\\s{0,})(?<BT_NAME>.{1,}))$");
                if (boost::regex_match(line, match, tmpl))
                {
                    tmp_sc.name = match.str("BT_NAME");
                    tmp_sc.addr = match.str("BT_MAC");
                    sc.push_back(tmp_sc);
                }
            }
            mutex_bt.unlock();
            return true;
        }
        catch (boost::exception &e)
        {
            syslog(LOG_ERR,"%s:error regex scan",LOG_NAME);
            mutex_bt.unlock();
            return false;
        }


    }
    mutex_bt.unlock();
    return false;
}

bool Fisgo_Bluetooth::RfcommBind(std::string ip)
{
    mutex_bt.lock();
    if(CheckHciRun() && CheckIp(ip))
    {
        EXTFE(system("rfcomm relese rfcomm0"),"%s:Error relese rfcomm");
        std::string command = "rfcomm bind /dev/rfcomm0 " + std::string(ip);
        EXTFE(system(command.c_str()),"%s:Error bind rfcomm");
        std::string data_out;
        SerchData(exec("rfcomm show rfcomm0"),data_out,REGEX_IP);
        mutex_bt.unlock();
        if(!strcmp(ip.c_str(), data_out.c_str()))
            return true;
    }
    mutex_bt.unlock();
    return false;
}

bool Fisgo_Bluetooth::DeviceGetInfo(DeviceInfo& sc)
{
    mutex_bt.lock();
    if(CheckHciRun() && CheckIp(sc.addr))
    {
        std::string command = "sdptool browse " + std::string(sc.addr);
        std::string result =  exec( command.c_str() );
        std::string data_out;
        char sep[2] = "\n";
        char *istr = strtok(&result[0], sep);
        while (istr != NULL)
        {

            if(SerchData(istr,data_out,"Service Name:"))
            {
                std::string someString(istr);
                boost::regex rx("Service Name: ");
                sc.serviceName.push_back(boost::regex_replace(someString, rx, "",boost::match_default | boost::format_perl));
            }
            else if(SerchData(istr,data_out,"UUID 128:"))
            {
                SerchData(istr, sc.uid,"[0-9a-fA-F]{8}(-[0-9a-fA-F]{4}){3}-[0-9a-fA-F]{12}");
            }
            istr = strtok(NULL, sep);
        }
        command = "hcitool inq |  grep '" + std::string(sc.addr) + "'";
        result = exec(command.c_str() );
        SerchData(result,data_out,"class: 0x[0-9a-fA-F]{6}");
        SerchData(data_out, data_out,"[0-9a-fA-F]{6}");
        std::stringstream ss;
        ss << std::hex << data_out;
        ss >> sc.dev_class ;
        mutex_bt.unlock();
        return true;
    }
    mutex_bt.unlock();
    return false;
}

bool Fisgo_Bluetooth::DeviceConnectStatus(std::string& ip)
{
    mutex_bt.lock();
    if(CheckHciRun())
    {
        std::string data_ip;
        if(SerchData(exec("rfcomm show rfcomm0"),data_ip,REGEX_IP))
        {
            std::string command_cc = "hcitool cc " + data_ip;
            std::string command_dc = "hcitool dc " + data_ip;
            EXTFE(system(command_cc.c_str()), "%s:hcitool cc");

            if(SerchData(exec("hcitool con"),ip,REGEX_IP))
            {
                EXTFE(system(command_dc.c_str()), "%s:hcitool dc");
                mutex_bt.unlock();
                if(!strcmp(ip.c_str(), data_ip.c_str()))
                    return true;
            }

        }
    }
    mutex_bt.unlock();
    return false;

}



bool Fisgo_Bluetooth::SendData(const char* ip,const char* data,int size)
{

    if(ip == NULL || data == NULL)
    {
        syslog(LOG_ERR,"%s:Invalid parameter for SendData",LOG_NAME);
        return false;
    }
    bool out = false;
    mutex_bt.lock();
    if(CheckHciRun() && CheckIp(ip))
    {

        struct sockaddr_rc addr = {0};
        int s, status;

        // allocate a socket
        s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

        // set the connection parameters (who to connect to)
        addr.rc_family = AF_BLUETOOTH;
        addr.rc_channel = (uint8_t) 1;
        str2ba(ip, &addr.rc_bdaddr);

        // connect to server
        status = connect(s, (struct sockaddr *) &addr, sizeof(addr));

        // send a message
        if (status == 0)
        {
            syslog(LOG_ERR,"%s:fail connect to socket",LOG_NAME);
            status = write(s, data, size);
            if(status > 0)
                out = true;
            else
                syslog(LOG_ERR,"%s:fail write to socket",LOG_NAME);
        }
        else
            syslog(LOG_ERR,"%s:fail connect to socket",LOG_NAME);


        close(s);

    }
    mutex_bt.unlock();
    return out;
}

bool Fisgo_Bluetooth::GetData(const char* buf,int& size)
{
    if(buf == NULL)
        return false;
    mutex_bt.lock();
    if(CheckHciRun())
    {
        struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
        int s, client, bytes_read;
        socklen_t opt = sizeof(rem_addr);

        // allocate socket
        s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

        // bind socket to port 1 of the first available
        // local bluetooth adapter
        loc_addr.rc_family = AF_BLUETOOTH;
        loc_addr.rc_bdaddr =((bdaddr_t) {{0, 0, 0, 0, 0, 0}});
        loc_addr.rc_channel = (uint8_t) 1;
        bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

        // put socket into listening mode
        listen(s, 1);

        // accept one connection
        client = accept(s, (struct sockaddr *)&rem_addr, &opt);

        memset((char*)buf, 0, size);

        // read data from the client
        bytes_read = read(client, (char*)buf, size);
        if( bytes_read > 0 ) {
           // printf("received [%s]\n", buff);
            size = bytes_read;
        }

        // close connection
        close(client);
        close(s);
        mutex_bt.unlock();
        return true;
    }
    mutex_bt.unlock();
    return false;
}



bool Fisgo_Bluetooth::Off()
{
    mutex_bt.lock();
    if(CheckHciRun() || CheckBluetoothdRun())
    {
        EXTFE(system("kill -9 $(pidof bluetoothd)"),"%s:Error killall bluetoothd");
        usleep(1000);
        if (CheckBluetoothdRun())
        {
            bool check_off_bluetooth = false;
            for(int i = 0;i<10;i++)
            {
                usleep(500);
                if(!CheckBluetoothdRun())
                {
                    check_off_bluetooth = true;
                    break;
                }
            }
            if(!check_off_bluetooth)
            {
                syslog(LOG_ERR, "%s:not off bluetoothd", LOG_NAME);
                mutex_bt.unlock();
                return false;
            }
        }
        EXTFE(system("hciconfig hci0 down"),"%s:Error hciconfig hci0 down");
        usleep(500);
        if (CheckHciRun())
        {
            syslog(LOG_ERR,"%s:not off bt",LOG_NAME);
            mutex_bt.unlock();
            return false;
        }
        EXTFE(system("rfcomm release rfcomm0"),"%s:Error rfcomm release ");
        mutex_bt.unlock();
        return true;
    }
    mutex_bt.unlock();
    return false;
}




