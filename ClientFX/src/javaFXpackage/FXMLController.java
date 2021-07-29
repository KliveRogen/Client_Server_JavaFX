/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package javaFXpackage;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URL;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.TextField;
import java.util.ResourceBundle;
import javafx.application.Platform;
import javafx.scene.control.CheckBox;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import network.Base64Convertion;
import network.CRC16;
import network.TCPConnection;
import network.TCPConnectionListener;

/**
 * FXML Controller class
 *
 * @author klive
 */
public class FXMLController implements Initializable, TCPConnectionListener {
    
    private TCPConnection connection;
    //private static final String IP_ADDR = "localhost";
    //private static final int PORT = 8888;
    
    @FXML
    private TextField myTextField;
    @FXML
    private TextArea myTextArea;
    @FXML
    private Button clearButton;
    @FXML
    private Button sendButton;
    @FXML
    private TextField fieldNickName;
    @FXML
    private Label labelStatus;
    @FXML
    private TextField ipField;
    @FXML
    private TextField portField;
    @FXML
    private Button disconnectButton;
    @FXML
    private Button connectButton;
    @FXML
    private CheckBox decodingCheckBox;
    
    /*Метод вызывается при нажатии кнопки "Отправить", отправляет сообщение на сервер*/
    @FXML
    private void sendMessage(ActionEvent event) throws UnsupportedEncodingException {
        //происходит отправка и вывод на экран сообщения, если была нажата кнопка "отправить", при этом поле сообщения не пустое и клиент подключен к серверу
        sendToServer();
    }
    
    /*Метод вызывается при нажатии кнопки "Очистить поле", очищает поле вывода*/
    @FXML
    private void clearScreen(ActionEvent event) {
        myTextArea.clear();
    }
    
    /*Метод вызывается при нажатии клавиши в поле ввода сообщения, отправляет сообщение на сервер*/
    @FXML
    private void keyPressedMessageField(KeyEvent event) {
        //происходит отправка и вывод на экран сообщения, если была нажата клавиша Enter, при этом  поле сообщения не пустое и клиент подключен к серверу
        if(event.getCode() == KeyCode.ENTER) {
            sendToServer();
        }    
    }
    
    /*Метод ощуествляет отключение от сервера при нажатии на кнопку "Отключиться"*/
    @FXML
    private void disconnectClick(ActionEvent event) {
        Platform.runLater(new Runnable() {
            @Override
            public void run() {
                //если соединение не создано - ничего не делаем
                if (connection == null) {
                    return;
                }
                //если соект клиента открыт - разрываем соединение
                if (!connection.isClose()) {
                    connection.disconnect();
                }
            }
        });
    }         
    
    /*Метод ощуествляет подключение к серверу с заданными IP и портом при нажатии на кнопку "Подключиться"*/
    @FXML
    private void connectClick(ActionEvent event) {
        Platform.runLater(new Runnable() {
            @Override
            public void run() {
                //если соединение не создано - создать соединение
                if (connection == null) {
                    startClient();
                    return;
                }
                //если соект клиента закрыт - создаем соединение
                if (connection.isClose()) {
                    startClient();
                }
            }
        });        
    }    
    
    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        myTextArea.setEditable(false); //блокировка поля вывода для редактирования
    }       
    
    /*Метод запускает соединение с сервером в отдельном потоке, при проблемме с соединением выдает исключение в окно ввода */
    public synchronized void startClient() {
        try {
            //запрос доступа на соединиение через данный порт и IP
            connection = new TCPConnection(this, ipField.getText(), Integer.parseInt(portField.getText()));
            connection.start();
        } catch (IOException e) {
            printMsgString("Возникло исключение при попытке соединения: " + e);
        }
    }    
    
    /*Метод отправки и вывода на экран сообщения из поля ввода, если поле сообщения не пустое и клиент подключен к серверу*/
    private synchronized void sendToServer() {
        if(connection == null) return;
        if (!myTextField.getText().isEmpty() && !connection.isClose()) {
            String msg = myTextField.getText(); //получение строки сообщения из текстового поля ввода
            printMsgString("(Исходное) : " + msg);//вывод введеного сообщения на поле вывода
            myTextField.clear(); //очистка поля ввода
            connection.sendString(encodeMessageAndCRC(fieldNickName.getText() + " : " +  msg)); //отправка сообщения с ником на сервер
        }
    }
    
    /*Метод кодирования сообщение в base64 и добавления к нему CRC16*/
    public byte[] encodeMessageAndCRC(String inputMessage) {
        byte[] encodedMessage = null;
        try {
            encodedMessage = Base64Convertion.encodeToByteArray(inputMessage); //кодирование в base64
            encodedMessage = CRC16.addCrc(encodedMessage); //добавление CRC16
        } catch (UnsupportedEncodingException ex) {
            System.out.println("Возникло исключение при кодировке: " + ex);
        }
        return encodedMessage;
    }    
    
    /*Метод вывода текста сообщения на поле вывода, выполняется из не-GUI потока*/
    private synchronized void printMsgString(String msg) {    
        Platform.runLater(new Runnable() {
            @Override
            public void run() {
                myTextArea.appendText(msg + "\n");  
            }
        });
    } 
    
    /*Метод изменения текста в labelStatus, выполняется из не-GUI потока*/
    private synchronized void printLabel(String msg) {    
        Platform.runLater(new Runnable() {
            @Override
            public void run() {
               labelStatus.setText(msg);  
            }
        });
    }   

    /*Метод выводит сообщение в поле вывода и изменяет labelStatus при соединениии с сервером*/ 
    @Override
    public void onConnectionReady(TCPConnection tcpConnection) {
        printMsgString("Соединение готово...");
        printLabel("клиент подключен к серверу " + connection.toString());
    }
    
    /*Метод декодирует принятый клиентом массив байт, проверяет CRC и выводит сообщение в поле вывода 
    При несовпадении CRC выводит в поле вывода сообщение об ошибке*/
    @Override
    public void onReceiveString(TCPConnection tcpConnection, byte [] inputArray) {
        String decodedString = null;
        if(CRC16.isCrcMatch(inputArray)) {
            try {
                inputArray = CRC16.removeCrc(inputArray); //удаление CRC из сообщения 
                //проверка значения чек-бокса (выбирается - декодировать сообщение или нет)
                if(decodingCheckBox.isSelected()) {
                    decodedString = Base64Convertion.decodeFromByteArray(inputArray); //декодирование сообщения и преобразование байт-массива в строку
                } else {
                    decodedString = Base64Convertion.convertEncodedMessage(inputArray);
                }
                System.out.println(decodedString); //вывод в консоль декодированного (или кодированного при отключенном CheckBox) сообщения
                printMsgString(decodedString);
            } catch (UnsupportedEncodingException ex) {
                System.out.println("Возникло исключение при декодировке: " + ex);
            }
        } else {
            printMsgString("Контрольные суммы принятого сообщения не совпадают!");
        }
    }
    
    /*Метод выводит сообщение в поле вывода и изменяет labelStatus при потере соединениия с сервером */
    @Override
    public void onDisconnect(TCPConnection tcpConnection) {
        printMsgString("Соединение закрыто...");
        printLabel("клиент не подключен к серверу");
    }
    
    /*Метод выводит сообщение в поле вывода и изменяет labelStatus при возникновении исключения*/
    @Override
    public void onException(TCPConnection tcpConnection, Exception e) {
        printMsgString("Возникло исключение в момент соединения : " + e);
        printLabel("клиент не подключен к серверу");
    }
}
