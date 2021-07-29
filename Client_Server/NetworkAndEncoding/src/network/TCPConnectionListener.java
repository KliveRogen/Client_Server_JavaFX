package network;
/**
 *
 * @author klive
 */
/*Интерфейс "слушателя" (для различной реализации приведенных методов для клиента и сервера)*/
public interface TCPConnectionListener {
    //виды событий:
    //соединение готово, далее с ним можно работать
    public void onConnectionReady(TCPConnection tcpConnection);
    //была принята строчка
    public void onReceiveString(TCPConnection tcpConnection, byte [] value);
    //произошло разъединение
    public void onDisconnect(TCPConnection tcpConnection);
    //возникло исключение
    public void onException(TCPConnection tcpConnection, Exception e);
}
