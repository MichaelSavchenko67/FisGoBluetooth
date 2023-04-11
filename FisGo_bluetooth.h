#ifndef FISGO_BLUETOOTH_LIBRARY_H
#define FISGO_BLUETOOTH_LIBRARY_H
#include <vector>
#include <string>
/**
 * @brief The DeviceInfo struct данные Bluetooth устройства
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
\brief Библиотека для работы с bt

Данная библиотека требует наличие утилит hciconfig, rfcomm, sdptool, hcitool и killall
*/
class Fisgo_Bluetooth
{

public:

/*!
Включить bt
*/
    static bool On();

/*!
Сканирование активных bt устройств
\param[in] sc Таблица найденных устройств
\param[inout] size Размер таблицы устройств
*/
    static bool Scan(std::vector<DeviceInfo>& sc);

/*!
Получение информации о bt устройства
\param[in] ip Адрес устройсва 17 байт
\param[out] ServiceName Таблица сервисных имен
\param[out] uid
\param[out] dev_class Класс устройства
*/
    static bool DeviceGetInfo(DeviceInfo& sc);
/*!
Получить информацию о состояние bt
*/
    static bool CheckRun();
/*!
Привязать bt устройство
\param[in] ip Адрес устройсва 17 байт
*/
    static bool RfcommBind(std::string ip);
/*!
Проверить статус подключения bt
\param[out] ip Адерес устройсва
*/
    static bool DeviceConnectStatus(std::string& ip);


/*!
Выключить bt
*/
    static bool Off();
private:
    /*!
Отправить данные по bt
\param[in] ip Адрес устройсва 17 байт
\param[in] data Массив данных
\param[in] size Размер данных
*/
    static bool SendData(const char *ip,const char *data, int size);

/*!
Прием данные по bt
\param[in] buf Массив данных
\param[inout] size Размер данных
*/
    static bool GetData(const char* buf,int& size);
};


#endif
