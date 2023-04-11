#ifndef FISGO_BLUETOOTH_LIBRARY_H
#define FISGO_BLUETOOTH_LIBRARY_H
#include <vector>
#include <string>
/**
 * @brief The DeviceInfo struct ����� Bluetooth ���ன�⢠
 */
struct DeviceInfo
{
    std::string addr;
    std::string name;
    std::vector<std::string> serviceName;
    std::string uid;
    uint32_t dev_class;
};
/*!
\brief ������⥪� ��� ࠡ��� � bt

������ ������⥪� �ॡ�� ����稥 �⨫�� hciconfig, rfcomm, sdptool, hcitool � killall
*/
class Fisgo_Bluetooth
{

public:

/*!
������� bt
*/
    static bool On();

/*!
�����஢���� ��⨢��� bt ���ன��
\param[in] sc ������ ��������� ���ன��
\param[inout] size ������ ⠡���� ���ன��
*/
    static bool Scan(std::vector<DeviceInfo>& sc);

/*!
����祭�� ���ଠ樨 � bt ���ன�⢠
\param[in] ip ���� ���னᢠ 17 ����
\param[out] ServiceName ������ �ࢨ��� ����
\param[out] uid
\param[out] dev_class ����� ���ன�⢠
*/
    static bool DeviceGetInfo(DeviceInfo& sc);
/*!
������� ���ଠ�� � ���ﭨ� bt
*/
    static bool CheckRun();
/*!
�ਢ易�� bt ���ன�⢮
\param[in] ip ���� ���னᢠ 17 ����
*/
    static bool RfcommBind(std::string ip);
/*!
�஢���� ����� ������祭�� bt
\param[out] ip ����� ���னᢠ
*/
    static bool DeviceConnectStatus(std::string& ip);


/*!
�몫���� bt
*/
    static bool Off();
private:
    /*!
��ࠢ��� ����� �� bt
\param[in] ip ���� ���னᢠ 17 ����
\param[in] data ���ᨢ ������
\param[in] size ������ ������
*/
    static bool SendData(const char *ip,const char *data, int size);

/*!
�ਥ� ����� �� bt
\param[in] buf ���ᨢ ������
\param[inout] size ������ ������
*/
    static bool GetData(const char* buf,int& size);
};


#endif
