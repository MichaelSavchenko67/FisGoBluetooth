//
// Created by pavel on 12/17/19.
//
//#include <cstdio>
#include <iostream>
#include <cstring>

#include "FisGo_bluetooth.h"

int main(int argc, char **argv)
{
    Fisgo_Bluetooth Bluetooth;
    int mode = atoi(argv[1]);
    switch(mode) {
        case 1:
            printf("BluetoothInit %d\n",Bluetooth.On());
            break;
        case 2:
            {
            std::vector<DeviceInfo>  SS;
            int size = 100;
            printf("BluetoothScan %d\n",Bluetooth.Scan(SS));
            printf("size = %d\n", SS.size());
            for (int i = 0; i < SS.size(); i++) {
              //  printf("%d ip=%s name=%s\n", i, SS[i].addr, SS[i].name);
                std::cout << i << " ip=" << SS[i].addr << " name=" << SS[i].name << "\n";
            }
        }
            break;
        case 3: {
            DeviceInfo deviceInfo;
            deviceInfo.addr = argv[2];
            printf("BluetoothDeviceGetInfo %d\n",
                   Bluetooth.DeviceGetInfo(deviceInfo));
            std::cout << " dev_class=" <<  deviceInfo.dev_class << "\n";
            for (int i = 0; i < deviceInfo.serviceName.size(); i++) {
                std::cout << i << " ServiceName=" <<  deviceInfo.serviceName[i] << "\n";
               // printf("%d ServiceName=%s\n", i, ServiceName[i]);
            }

                std::cout << " uid=" <<  deviceInfo.uid << "\n";
                //printf("%d uid=%s\n", i, uid[i]);
        }
            break;
        case 4:
        {
            printf("BluetoothRfcommBind %d\n",Bluetooth.RfcommBind(argv[2]));

        }
            break;
        case 5:
        {
            std::string  ip;
            printf("BluetoothDeviceConnectStatus\n");
            if(Bluetooth.DeviceConnectStatus(ip))
                std::cout << " connect:" <<  ip << "\n";
            else
                printf("not connect\n");
        }
            break;
        case 6:
        {
            printf("BluetoothCheckRun\n");
            printf("CheckRun %d\n",Bluetooth.CheckRun());
        }
            break;
        case 7:
        {
            int col = atoi(argv[2]);
            bool test = true;
            for (int i =0;i< col;i++)
            {
               /* bool on = Bluetooth.On();
                bool cr = Bluetooth.CheckRun();
                bool off = Bluetooth.Off();
                if(!on || !cr || !off)
                {
                    test = false;
                    printf("error test %d %d %d!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", on, cr, off);
                    break;
                }
                if(test)
                    printf("test ok\n");*/
                bool on = Bluetooth.On();
                std::vector<DeviceInfo>  SS;
                bool sc = Bluetooth.Scan(SS);
                DeviceInfo deviceInfo;
                deviceInfo.addr = SS[0].addr;
                bool bn = Bluetooth.RfcommBind(deviceInfo.addr);
                bool gt = Bluetooth.DeviceGetInfo(deviceInfo);
                bool off = Bluetooth.Off();

                if(!on || !gt || !bn || !off || !sc)
                {
                    test = false;
                    printf("error test %d %d %d %d %d!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", on,sc,bn,gt, off);
                    break;
                }
                if(test)
                    printf("test ok sc\n");

            }



        }
            break;
     /*   case 6:
        {
            printf("BluetoothSendData %d\n",Bluetooth.SendData(argv[2],argv[3],strlen(argv[3])+1));

        }
            break;
        case 7:
        {
            char buf[400];
            int size = 400;
            printf("BluetoothGetData %d\n",Bluetooth.GetData(buf,size));

            printf("size = %d data=\n%s\n",size,buf);

        }
            break;*/
        case 8:
        {
            printf("BluetoothDeInit %d\n",Bluetooth.Off());

        }
            break;

    }
    return 0;
}
