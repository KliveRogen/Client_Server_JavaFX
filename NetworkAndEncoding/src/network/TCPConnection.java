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
//класс конкретного TCP соеднинения
public class TCPConnection extends Thread{

    private final Socket socket; //сокет, связаный с данным TCP соединением
    private final TCPConnectionListener eventListener;//слушататель событий
    private final DataInputStream in; //поток ввода (выдает строку)
    private final DataOutputStream out;//поток вывода (принимает строку)
    
    /*Метод отсылает массив байт в поток вывода (принимает только само сообщение)*/
    private void sendBytes(byte[] myByteArray) throws IOException {
        sendBytes(myByteArray, 0, myByteArray.length);
    }

    /*Метод отсылает массив байт в поток вывода (принимает сообщение, его начало и конец)*/
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
    
    /*Метод ожидает прихода байт из потока ввода и выдает принятые байты */
    private byte[] readBytes() throws IOException {
        // Again, probably better to store these objects references in the support class
        int len = in.readInt();
        byte[] data = new byte[len];
        if (len > 0) {
            in.readFully(data);
        }
        return data;
    }   
    
    /*Конструктор TCP соеднинения для клиента, принимающий объект TCPConnectionListener (в данном случае его дочерние классы),
    IP-адрес и порт подключения (уже внутри передается объект класса Socket)
    */
    public TCPConnection(TCPConnectionListener eventListener, String ipAddr, int port) throws IOException{
        this(eventListener, new Socket(ipAddr, port));//вызов другого конструктора
    }
    /*Конструктор TCP соеднинения для сервера, 
    принимает объект TCPConnectionListener (в данном случае его дочерние классы) и уже готовый объект Socket
    */
    public TCPConnection(TCPConnectionListener eventListener, Socket socket) throws IOException{
        this.eventListener = eventListener;
        this.socket = socket;
        in = new DataInputStream(socket.getInputStream());
        out = new DataOutputStream(socket.getOutputStream());
        
    }
    
    /*Переопределение метода run класса Thread
    Данная нить(поток) "слушает" входящее соединение (читает поток ввода)
    При приеме строчки генерирует событие onReceiveString
    При создании соединения - событие onConnectionReady
    При разъединении - событие onDisconnect
    При возникновении исключения - событие onException
    */
    @Override
    public void run(){
        try{
            //если поток запустился, то соединение было создано => вызываем событие onConnectionReady
            eventListener.onConnectionReady(this);
            //если поток не прерван, БЕСКОНЕЧНО читаем строку входного соединения 
            while(!this.isInterrupted()){
                //если строка из потока ввода прочитана, вызывается событие onReceiveString
                eventListener.onReceiveString(this, readBytes());
            }
        }catch(IOException e){
            eventListener.onException(this, e);
            
        }finally{
            //если цикл завершился или поток прервался, вызывается событие onDisconnect
            eventListener.onDisconnect(this);
        }
    }

    /*Метод отправки сообщения серверу/клиенту*/
    public synchronized void sendString(byte[] value){
        try {
            //запись строки в поток вывода
            //out.write(value + "\r\n");
            //out.flush();//сброс и отправка буфера
            sendBytes(value);
            
        } catch (IOException e) {
            eventListener.onException(TCPConnection.this, e);
            disconnect(); //если строчку передать не удалось - обрываем соединение
        }
    }
    
    /*Метод обрывания соединения*/
    public synchronized void disconnect(){
        this.interrupt();//прерывание потока данного TCP соединения
        try {
            socket.close();//закрытие сокета
            in.close();
            out.close();
        } catch (IOException e) {
            eventListener.onException(TCPConnection.this, e);
        }
    }
    
    /*Метод передачи состояния сокета*/
    public synchronized boolean isClose(){
        return socket.isClosed();
    }
    
    /*Переписывание метода toString для сообщений о подключениях клиентов 
    (выдает строку вида "IP:Port")
    */
    @Override
    public String toString(){
        return socket.getInetAddress().toString() + ":" + socket.getPort();
    }
    
    
}
