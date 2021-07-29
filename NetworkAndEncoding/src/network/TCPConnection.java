/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package network;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;

/**
 *
 * @author klive
 */
//����� ����������� TCP �����������
public class TCPConnection extends Thread{

    private final Socket socket; //�����, �������� � ������ TCP �����������
    private final TCPConnectionListener eventListener;//����������� �������
    private final DataInputStream in; //����� ����� (������ ������)
    private final DataOutputStream out;//����� ������ (��������� ������)
    
    /*����� �������� ������ ���� � ����� ������ (��������� ������ ���� ���������)*/
    private void sendBytes(byte[] myByteArray) throws IOException {
        sendBytes(myByteArray, 0, myByteArray.length);
    }

    /*����� �������� ������ ���� � ����� ������ (��������� ���������, ��� ������ � �����)*/
    private void sendBytes(byte[] myByteArray, int start, int len) throws IOException {
        if (len < 0)
            throw new IllegalArgumentException("Negative length not allowed");
        if (start < 0 || start >= myByteArray.length)
            throw new IndexOutOfBoundsException("Out of bounds: " + start);
        // Other checks if needed.
        // May be better to save the streams in the support class;
        // just like the socket variable.
        out.writeInt(len);
        if (len > 0) {
            out.write(myByteArray, start, len);
        }
    }    
    
    /*����� ������� ������� ���� �� ������ ����� � ������ �������� ����� */
    private byte[] readBytes() throws IOException {
        // Again, probably better to store these objects references in the support class
        int len = in.readInt();
        byte[] data = new byte[len];
        if (len > 0) {
            in.readFully(data);
        }
        return data;
    }   
    
    /*����������� TCP ����������� ��� �������, ����������� ������ TCPConnectionListener (� ������ ������ ��� �������� ������),
    IP-����� � ���� ����������� (��� ������ ���������� ������ ������ Socket)
    */
    public TCPConnection(TCPConnectionListener eventListener, String ipAddr, int port) throws IOException{
        this(eventListener, new Socket(ipAddr, port));//����� ������� ������������
    }
    /*����������� TCP ����������� ��� �������, 
    ��������� ������ TCPConnectionListener (� ������ ������ ��� �������� ������) � ��� ������� ������ Socket
    */
    public TCPConnection(TCPConnectionListener eventListener, Socket socket) throws IOException{
        this.eventListener = eventListener;
        this.socket = socket;
        in = new DataInputStream(socket.getInputStream());
        out = new DataOutputStream(socket.getOutputStream());
        
    }
    
    /*��������������� ������ run ������ Thread
    ������ ����(�����) "�������" �������� ���������� (������ ����� �����)
    ��� ������ ������� ���������� ������� onReceiveString
    ��� �������� ���������� - ������� onConnectionReady
    ��� ������������ - ������� onDisconnect
    ��� ������������� ���������� - ������� onException
    */
    @Override
    public void run(){
        try{
            //���� ����� ����������, �� ���������� ���� ������� => �������� ������� onConnectionReady
            eventListener.onConnectionReady(this);
            //���� ����� �� �������, ���������� ������ ������ �������� ���������� 
            while(!this.isInterrupted()){
                //���� ������ �� ������ ����� ���������, ���������� ������� onReceiveString
                eventListener.onReceiveString(this, readBytes());
            }
        }catch(IOException e){
            eventListener.onException(this, e);
            
        }finally{
            //���� ���� ���������� ��� ����� ���������, ���������� ������� onDisconnect
            eventListener.onDisconnect(this);
        }
    }

    /*����� �������� ��������� �������/�������*/
    public synchronized void sendString(byte[] value){
        try {
            //������ ������ � ����� ������
            //out.write(value + "\r\n");
            //out.flush();//����� � �������� ������
            sendBytes(value);
            
        } catch (IOException e) {
            eventListener.onException(TCPConnection.this, e);
            disconnect(); //���� ������� �������� �� ������� - �������� ����������
        }
    }
    
    /*����� ��������� ����������*/
    public synchronized void disconnect(){
        this.interrupt();//���������� ������ ������� TCP ����������
        try {
            socket.close();//�������� ������
            in.close();
            out.close();
        } catch (IOException e) {
            eventListener.onException(TCPConnection.this, e);
        }
    }
    
    /*����� �������� ��������� ������*/
    public synchronized boolean isClose(){
        return socket.isClosed();
    }
    
    /*������������� ������ toString ��� ��������� � ������������ �������� 
    (������ ������ ���� "IP:Port")
    */
    @Override
    public String toString(){
        return socket.getInetAddress().toString() + ":" + socket.getPort();
    }
    
    
}
