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
//в классе ServerNew реализуется интерфейс TCPConnectionListener
public class ServerNew implements TCPConnectionListener {

    public static void main(String[] args) {
        int serverPort = getPort(args);// serverPort - порт, который "слушает" сервер (к нему обращается клиент)
        System.out.println("Установлен порт: " + serverPort);
        new ServerNew().startServer(serverPort); //создание объекта сервера и его запуск   
    }
    //Создание списка TCP соединений (TCPConnections)
    private final ArrayList<TCPConnection> connections = new ArrayList<>();

    /*Метод запуска сервера*/
    public void startServer(int serverPort) {
        
        System.out.println("Сервер запущен...");//сообщения о запуске серера
        //создание сокета сервера с определенным портом
        try(ServerSocket serverSocket = new ServerSocket(serverPort)) {
            while(true) {
                try {
                    //для каждого нового соединения создается экземпляр TCPConnection, который получает сокет клиента
                    //экземпляр создается только после соединения с клиентом
                    //(при этом запускается отдельный поток)
                    new TCPConnection(this, serverSocket.accept()).start();
                } catch(IOException e) {
                    System.err.println("Возникло исключение приеме сокета клиента: " + e);
                }
            }
        } catch(IOException e) {
            throw new RuntimeException(e);
        }
    }
    
    /*Метод выводит сообщение в консоль и отправляет его всем клиентам при подсоединениии клиентам*/ 
    @Override
    public synchronized void onConnectionReady(TCPConnection tcpConnection) {
        //если соединение готово - оно добавляется в список соединений
        connections.add(tcpConnection);
        //sendToAllConnections("Клиент " + tcpConnection.toString() + " подключился");
        System.out.println("Клиент " + tcpConnection.toString() + " подключился");
        sendToAllConnections("Клиент " + tcpConnection.toString() + " подключился");
    }
    
    /*Метод декодирует проверяет CRC принятого сообщения, декодирует его из base64, 
    при отсутствии ника добавляет в начало ip и порт клиента, само сообщение переводит в верхний регистр
    При несовпадении CRC выводит в консоль сервера сообщение об ошибке*/ 
    @Override
    public synchronized void onReceiveString(TCPConnection tcpConnection, byte [] inputArray) {
        //при приеме строки она отправляется всем клиентам
        String decodedString = null;
        if(CRC16.isCrcMatch(inputArray)) {
            try {
                inputArray = CRC16.removeCrc(inputArray); //удаление CRC из сообщения 
                decodedString = Base64Convertion.decodeFromByteArray(inputArray); //декодирование сообщения и преобразование байт-массива в строку
                System.out.println(decodedString); //вывод в консоль декодированного сообщения
            } catch (UnsupportedEncodingException ex) {
                System.out.println("Возникло исключение при декодировке: " + ex);
            }
            //проверка принятого сообщения на наличие ": ". Если оно есть, то разделяем строку на массивы.
            if (decodedString.contains(": ")) {
                String[] parts = decodedString.split("(?=:)"); //выражение с : будет в правой части
                int partsSize = parts.length;
                //при наличии ника ничего с ним не делаем. При отсутствии - ником указываем ip и порт клиента.
                if (parts[0].trim().length() == 0) {
                    parts[0] = tcpConnection.toString().substring(1) + " ";
                }
                parts[partsSize-1] = parts[partsSize-1].toUpperCase(); //перевод сообщения в верхний регистр
                //склеивание "частей" сообщения
                StringBuilder strBuilder = new StringBuilder();
                for (String part : parts) {
                    strBuilder.append(part);
                }
                decodedString = strBuilder.toString();
                sendToAllConnections(decodedString);
            }
        } else {
            System.out.println("Контрольные суммы принятого сообщения не совпадают!");
        }
    }

    /*Метод декодирует проверяет CRC принятого сообщения, декодирует его из base64 в консоль и отправляет его всем клиентам при отключении клиента*/ 
    @Override
    public synchronized void onDisconnect(TCPConnection tcpConnection) {
        connections.remove(tcpConnection);//удаление соединения из списка при разрыве связи с ним
        //sendToAllConnections("Клиент " + tcpConnection.toString() + " подключился");
        System.out.println("Клиент " + tcpConnection.toString() + " отключился");
        sendToAllConnections("Клиент " + tcpConnection.toString() + " отключился");
    }
    
    /*Метод выводит сообщение об ошибке при возникновении исключения в соединении*/ 
    @Override
    public synchronized void onException(TCPConnection tcpConnection, Exception e) {
        System.err.println("TCPConnection исключение: " + e);
    }
    
    /*Метод кодирует сообщение в base64, добавляет CRC16 и  отправляет его всем подключенным клиентам*/
    private void sendToAllConnections(String value) {

        final int connectionSize = connections.size();//количество соединений
        for (int i = 0; i < connectionSize; i++) {
            connections.get(i).sendString(encodeMessageAndCRC(value));//отправка сообщения клиенту i 
        }
    }
    
    /*Метод кодирует сообщение в base64, добавляет CRC16*/
    private byte[] encodeMessageAndCRC(String inputMessage){
        byte[] encodedMessage = null;
        try {
            encodedMessage = Base64Convertion.encodeToByteArray(inputMessage); //кодирование в base64
            encodedMessage = CRC16.addCrc(encodedMessage); //добавление CRC16
        } catch (UnsupportedEncodingException ex) {
            System.out.println("Возникло исключение при кодировке: " + ex);
        }
        return encodedMessage;
    }
    
    /*Проверка, что строка является числом Int*/
    private static boolean isNumber(String str) {
        if (str == null || str.isEmpty()) return false;
        for (int i = 0; i < str.length(); i++) {
            if (!Character.isDigit(str.charAt(i))) return false;
        }
        return true;
    }
    
    /*Получение порта сервера из массива аргументов*/
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
