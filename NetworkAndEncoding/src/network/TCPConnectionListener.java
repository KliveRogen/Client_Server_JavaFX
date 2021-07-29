package network;
/**
 *
 * @author klive
 */
/*��������� "���������" (��� ��������� ���������� ����������� ������� ��� ������� � �������)*/
public interface TCPConnectionListener {
    //���� �������:
    //���������� ������, ����� � ��� ����� ��������
    public void onConnectionReady(TCPConnection tcpConnection);
    //���� ������� �������
    public void onReceiveString(TCPConnection tcpConnection, byte [] value);
    //��������� ������������
    public void onDisconnect(TCPConnection tcpConnection);
    //�������� ����������
    public void onException(TCPConnection tcpConnection, Exception e);
}
