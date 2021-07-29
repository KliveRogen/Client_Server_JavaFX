/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package serverPackage;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.ServerSocket;
import java.util.ArrayList;
import network.Base64Convertion;
import network.CRC16;
import network.TCPConnection;
import network.TCPConnectionListener;

/**
 *
 * @author klive
 */
//� ������ ServerNew ����������� ��������� TCPConnectionListener
public class ServerNew implements TCPConnectionListener {

    public static void main(String[] args) {
        int serverPort = getPort(args);// serverPort - ����, ������� "�������" ������ (� ���� ���������� ������)
        System.out.println("���������� ����: " + serverPort);
        new ServerNew().startServer(serverPort); //�������� ������� ������� � ��� ������   
    }
    //�������� ������ TCP ���������� (TCPConnections)
    private final ArrayList<TCPConnection> connections = new ArrayList<>();

    /*����� ������� �������*/
    public void startServer(int serverPort) {
        
        System.out.println("������ �������...");//��������� � ������� ������
        //�������� ������ ������� � ������������ ������
        try(ServerSocket serverSocket = new ServerSocket(serverPort)) {
            while(true) {
                try {
                    //��� ������� ������ ���������� ��������� ��������� TCPConnection, ������� �������� ����� �������
                    //��������� ��������� ������ ����� ���������� � ��������
                    //(��� ���� ����������� ��������� �����)
                    new TCPConnection(this, serverSocket.accept()).start();
                } catch(IOException e) {
                    System.err.println("�������� ���������� ������ ������ �������: " + e);
                }
            }
        } catch(IOException e) {
            throw new RuntimeException(e);
        }
    }
    
    /*����� ������� ��������� � ������� � ���������� ��� ���� �������� ��� �������������� ��������*/ 
    @Override
    public synchronized void onConnectionReady(TCPConnection tcpConnection) {
        //���� ���������� ������ - ��� ����������� � ������ ����������
        connections.add(tcpConnection);
        //sendToAllConnections("������ " + tcpConnection.toString() + " �����������");
        System.out.println("������ " + tcpConnection.toString() + " �����������");
        sendToAllConnections("������ " + tcpConnection.toString() + " �����������");
    }
    
    /*����� ���������� ��������� CRC ��������� ���������, ���������� ��� �� base64, 
    ��� ���������� ���� ��������� � ������ ip � ���� �������, ���� ��������� ��������� � ������� �������
    ��� ������������ CRC ������� � ������� ������� ��������� �� ������*/ 
    @Override
    public synchronized void onReceiveString(TCPConnection tcpConnection, byte [] inputArray) {
        //��� ������ ������ ��� ������������ ���� ��������
        String decodedString = null;
        if(CRC16.isCrcMatch(inputArray)) {
            try {
                inputArray = CRC16.removeCrc(inputArray); //�������� CRC �� ��������� 
                decodedString = Base64Convertion.decodeFromByteArray(inputArray); //������������� ��������� � �������������� ����-������� � ������
                System.out.println(decodedString); //����� � ������� ��������������� ���������
            } catch (UnsupportedEncodingException ex) {
                System.out.println("�������� ���������� ��� �����������: " + ex);
            }
            //�������� ��������� ��������� �� ������� ": ". ���� ��� ����, �� ��������� ������ �� �������.
            if (decodedString.contains(": ")) {
                String[] parts = decodedString.split("(?=:)"); //��������� � : ����� � ������ �����
                int partsSize = parts.length;
                //��� ������� ���� ������ � ��� �� ������. ��� ���������� - ����� ��������� ip � ���� �������.
                if (parts[0].trim().length() == 0) {
                    parts[0] = tcpConnection.toString().substring(1) + " ";
                }
                parts[partsSize-1] = parts[partsSize-1].toUpperCase(); //������� ��������� � ������� �������
                //���������� "������" ���������
                StringBuilder strBuilder = new StringBuilder();
                for (String part : parts) {
                    strBuilder.append(part);
                }
                decodedString = strBuilder.toString();
                sendToAllConnections(decodedString);
            }
        } else {
            System.out.println("����������� ����� ��������� ��������� �� ���������!");
        }
    }

    /*����� ���������� ��������� CRC ��������� ���������, ���������� ��� �� base64 � ������� � ���������� ��� ���� �������� ��� ���������� �������*/ 
    @Override
    public synchronized void onDisconnect(TCPConnection tcpConnection) {
        connections.remove(tcpConnection);//�������� ���������� �� ������ ��� ������� ����� � ���
        //sendToAllConnections("������ " + tcpConnection.toString() + " �����������");
        System.out.println("������ " + tcpConnection.toString() + " ����������");
        sendToAllConnections("������ " + tcpConnection.toString() + " ����������");
    }
    
    /*����� ������� ��������� �� ������ ��� ������������� ���������� � ����������*/ 
    @Override
    public synchronized void onException(TCPConnection tcpConnection, Exception e) {
        System.err.println("TCPConnection ����������: " + e);
    }
    
    /*����� �������� ��������� � base64, ��������� CRC16 �  ���������� ��� ���� ������������ ��������*/
    private void sendToAllConnections(String value) {

        final int connectionSize = connections.size();//���������� ����������
        for (int i = 0; i < connectionSize; i++) {
            connections.get(i).sendString(encodeMessageAndCRC(value));//�������� ��������� ������� i 
        }
    }
    
    /*����� �������� ��������� � base64, ��������� CRC16*/
    private byte[] encodeMessageAndCRC(String inputMessage){
        byte[] encodedMessage = null;
        try {
            encodedMessage = Base64Convertion.encodeToByteArray(inputMessage); //����������� � base64
            encodedMessage = CRC16.addCrc(encodedMessage); //���������� CRC16
        } catch (UnsupportedEncodingException ex) {
            System.out.println("�������� ���������� ��� ���������: " + ex);
        }
        return encodedMessage;
    }
    
    /*��������, ��� ������ �������� ������ Int*/
    private static boolean isNumber(String str) {
        if (str == null || str.isEmpty()) return false;
        for (int i = 0; i < str.length(); i++) {
            if (!Character.isDigit(str.charAt(i))) return false;
        }
        return true;
    }
    
    /*��������� ����� ������� �� ������� ����������*/
    private static int getPort(String[] args){
        int portServer;
        if (args.length > 0 && isNumber(args[0])) {
            portServer = Integer.parseInt(args[0]);
        } else {
            portServer = 8888;
        }
        return portServer;
    }
}
