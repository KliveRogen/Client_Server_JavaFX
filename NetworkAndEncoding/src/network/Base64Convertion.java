/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package network;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;

/**
 *
 * @author klive
 */
public class Base64Convertion {
    
    /*Таблица для кодирования в base64*/
    private static final char[] encodingTable = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };
    
    /*Создание таблицы для декодирования из base64 (для метода decodeFromString)*/
    private static final int[] decodingTable = new int[256];
    static {
        Arrays.fill(decodingTable, -1);
        for (int i = 0; i < encodingTable.length; i++)
            decodingTable[encodingTable[i]] = i;
        decodingTable['='] = -2;
    }
    
    /**
     * Метод кодирования строки в строку base64
     * @param inputMessage исходное сообщение в виде строки
     * @return encodeMessage - кодированное в base64 сообщение в виде строки
     * @throws java.io.UnsupportedEncodingException
     */
    public static String encodeToString (String inputMessage) throws UnsupportedEncodingException{
        try{
            byte[] dataArray = inputMessage.getBytes("UTF-8"); //входное сообщение в виде массива байтов
            int originalByteNumber; //номер элемента оригнального байтового массива
            int appendSize; //определяется количество добавляемых символов
            int dataEncodeLength; // длина закодированного сообщения в битах
            int clearLength; //количество "чистых" байт (кратных трем)
            String outMessage = new String(); //выходное сообщение
            switch (dataArray.length % 3){
                case (1):
                    appendSize = 2; //нужно добавить ==
                    break;
                case(2):
                    appendSize = 1; //нужно добавить =
                    break;
                case(0):
                    appendSize = 0; //все прекрасно
                    break;
                default:
                    appendSize = -1; //произошла ошибка
                    break;         
            }
            //длина закодированного сообщения
            dataEncodeLength = (dataArray.length * 8 + appendSize * 2) /  6 + appendSize;
            clearLength = (dataArray.length / 3) * 3;
            byte[] outputDataArray = new byte [dataEncodeLength]; //байтовый массив с сообщениями, кодированными с помощью base64
            int encodedByteNumber = 0; //номер элемента кодированного байтового массива
            //обработка оригинальных байтов, по номеру < clearLength (кратно 3)
            for (originalByteNumber = 0; originalByteNumber < clearLength; originalByteNumber += 3){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //обработка первого байта из четверки
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber] & 0b11) << 4) | ((dataArray[originalByteNumber + 1] >> 4) & 15)); //обработка второго байта из четверки
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber + 1] & 0b1111) << 2) | ((dataArray[originalByteNumber + 2] >> 6)  & 3)); //обработка третьего байта из четверки
                outputDataArray[encodedByteNumber++] = (byte) (dataArray[originalByteNumber + 2] & 0b111111); //обработка четвертого байта из четверки
            }
            //обработка оригинальных байтов, по номеру >= clearLength
            if(appendSize == 1){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //обработка первого байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber] & 0b11) << 4) | ((dataArray[originalByteNumber + 1] >> 4)) & 15); //обработка второго байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber + 1] & 0b1111) << 2)); //обработка третьего байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = 0; //обработка четвертого байта из оставшейся четверки
            }else if(appendSize == 2){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //обработка первого байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] & 0b11) << 4); //обработка второго байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = 0; //обработка третьего байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = 0; //обработка четвертого байта из оставшейся четверки
            }      

            for (int i = 0; i < dataEncodeLength; i++){
                if ((i+1) > (dataEncodeLength - appendSize)){
                    outMessage += "=";        
                }else{
                    outMessage += encodingTable[outputDataArray[i]]; 
                }
            }
            return outMessage;
        }catch (UnsupportedEncodingException | ArrayIndexOutOfBoundsException ex){
            System.err.println("Непредвиденная ошибка при кодировании");
            return ""; 
        }
    }

    /**
     * Метод кодирования строки в массив байт base64
     * @param inputMessage исходное сообщение в виде строки
     * @return encodeMessage - кодированное в base64 сообщение в виде массива байт
     * @throws java.io.UnsupportedEncodingException
     */
    public static byte[] encodeToByteArray (String inputMessage) throws UnsupportedEncodingException{
        try{
            byte[] dataArray = inputMessage.getBytes("UTF-8"); //входное сообщение в виде массива байтов
            int originalByteNumber; //номер элемента оригнального байтового массива
            int appendSize; //определяется количество добавляемых символов
            int dataEncodeLength; // длина закодированного сообщения в битах
            int clearLength; //количество "чистых" байт (кратных трем)
            switch (dataArray.length % 3){
                case (1):
                    appendSize = 2; //нужно добавить ==
                    break;
                case(2):
                    appendSize = 1; //нужно добавить =
                    break;
                case(0):
                    appendSize = 0; //все прекрасно
                    break;
                default:
                    appendSize = -1; //произошла ошибка
                    break;         
            }
            //длина закодированного сообщения
            dataEncodeLength = (dataArray.length * 8 + appendSize * 2) /  6 + appendSize;
            clearLength = (dataArray.length / 3) * 3;
            byte[] outputDataArray = new byte [dataEncodeLength]; //байтовый массив с сообщениями, кодированными с помощью base64
            int encodedByteNumber = 0; //номер элемента кодированного байтового массива
            //обработка оригинальных байтов, по номеру < clearLength (кратно 3)
            for (originalByteNumber = 0; originalByteNumber < clearLength; originalByteNumber += 3){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //обработка первого байта из четверки
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber] & 0b11) << 4) | ((dataArray[originalByteNumber + 1] >> 4) & 15)); //обработка второго байта из четверки
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber + 1] & 0b1111) << 2) | ((dataArray[originalByteNumber + 2] >> 6)  & 3)); //обработка третьего байта из четверки
                outputDataArray[encodedByteNumber++] = (byte) (dataArray[originalByteNumber + 2] & 0b111111); //обработка четвертого байта из четверки
            }
            //обработка оригинальных байтов, по номеру >= clearLength
            if(appendSize == 1){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //обработка первого байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber] & 0b11) << 4) | ((dataArray[originalByteNumber + 1] >> 4)) & 15); //обработка второго байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber + 1] & 0b1111) << 2)); //обработка третьего байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = -2; //обработка четвертого байта из оставшейся четверки (вместо лишних нулей будет -2)
            }else if(appendSize == 2){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //обработка первого байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] & 0b0011) << 4); //обработка второго байта из оставшейся четверки
                outputDataArray[encodedByteNumber++] = -2; //обработка третьего байта из оставшейся четверки (вместо лишних нулей будет -2)
                outputDataArray[encodedByteNumber++] = -2; //обработка четвертого байта из оставшейся четверки (вместо лишних нулей будет -2)
            }
            return outputDataArray;
        }catch (UnsupportedEncodingException | ArrayIndexOutOfBoundsException ex){
            System.err.println("Непредвиденная ошибка при кодировании");
            byte[] outputDataArray = new byte [] {0};
            return outputDataArray; 
        }
    }
    
    /**
     * Метод декодирования строки из base64 в строку
     * @param encodeMessage кодированное в base64 сообщение в виде строки
     * @return decodeMessage - декодированное из base64 сообщение в виде строки
     * @throws java.io.UnsupportedEncodingException
     */
    public static String decodeFromString (String encodeMessage) throws UnsupportedEncodingException{
        try{
            int encodeLength = encodeMessage.length();
            int countOfEqualSign = encodeMessage.endsWith("==") ? 2 : encodeMessage.endsWith("=") ? 1 : 0; //количество знаков "равно" в конце
            int decodeArraySize = (encodeLength - countOfEqualSign) * 6 / 8; //длина исходного сообщения
            byte[] encodeArray = encodeMessage.getBytes("UTF-8");
            byte[] decodeArray = new byte[decodeArraySize];
            int encodedByteNumber; //номер байта закодированного сообщения
            int decodedByteNumber = 0; //номер байта раскодированного сообщения
            for (encodedByteNumber = 0; encodedByteNumber < encodeLength; encodedByteNumber += 4){
                decodeArray[decodedByteNumber++] = (byte) ((decodingTable[encodeArray[encodedByteNumber]] << 2) | (decodingTable[encodeArray[encodedByteNumber + 1]] >> 4)); //первый байт из тройки
                if (decodedByteNumber == decodeArraySize) break;
                decodeArray[decodedByteNumber++] = (byte) ((decodingTable[encodeArray[encodedByteNumber + 1]] << 4) | (decodingTable[encodeArray[encodedByteNumber + 2]] >> 2)); //второй байт из тройки
                if (decodedByteNumber == decodeArraySize ) break;
                decodeArray[decodedByteNumber++] = (byte) ((decodingTable[encodeArray[encodedByteNumber + 2]] << 6) | (decodingTable[encodeArray[encodedByteNumber + 3]])); //третий байт из тройки   
            }
            String decodedString = new String(decodeArray, "UTF-8");
            return decodedString;
        }catch (UnsupportedEncodingException | ArrayIndexOutOfBoundsException ex){
            System.err.println("Непредвиденная ошибка при декодировании");
            return ""; 
        }
    }
    
    /**
     * Метод декодирования массива байт из base64 в строку
     * @param encodeArray кодированное в base64 сообщение в массива байи
     * @return decodeMessage - декодированное из base64 сообщение в виде строки
     * @throws java.io.UnsupportedEncodingException
     */
    public static String decodeFromByteArray (byte[] encodeArray) throws UnsupportedEncodingException{
        int encodeLength = encodeArray.length;
        int encodedByteNumber; //номер байта закодированного сообщения
        int decodedByteNumber = 0; //номер байта раскодированного сообщения
        int decodeArraySize; //длина исходного сообщения
        int countOfEqualSign = 0; //количество знаков "равно" в конце
        if (encodeArray[encodeLength-1] == -2){
            countOfEqualSign = 1;
            if (encodeArray[encodeLength-2] == -2){
               countOfEqualSign = 2; 
            }
        }
        decodeArraySize = (encodeLength - countOfEqualSign) * 6 / 8; //длина исходного сообщения
        byte[] decodeArray = new byte[decodeArraySize];
        for (encodedByteNumber = 0; encodedByteNumber < encodeLength; encodedByteNumber += 4){
            decodeArray[decodedByteNumber++] = (byte) ((encodeArray[encodedByteNumber] << 2) | (encodeArray[encodedByteNumber + 1] >> 4)); //первый байт из тройки
            if (decodedByteNumber == decodeArraySize) break;
            decodeArray[decodedByteNumber++] = (byte) ((encodeArray[encodedByteNumber + 1] << 4) | (encodeArray[encodedByteNumber + 2] >> 2)); //второй байт из тройки
            if (decodedByteNumber == decodeArraySize ) break;
            decodeArray[decodedByteNumber++] = (byte) ((encodeArray[encodedByteNumber + 2] << 6) | (encodeArray[encodedByteNumber + 3])); //третий байт из тройки   
        }
        String decodedString = new String(decodeArray, "UTF-8");
        return decodedString;
    }
    
    /**
     * Метод преобразования кодированного массива байт в кодированную строку
     * @param inputArray кодированный в base64 массив байт
     * @return outMessage - кодированная в base64 строка
     */
    public static String convertEncodedMessage(byte [] inputArray){
        String outMessage = "";
        int arraySize = inputArray.length;
        for (int i = 0; i < arraySize; i++){
            if (inputArray[i] == -2){
                outMessage += "=";        
            }else{
                outMessage += encodingTable[inputArray[i]]; 
            }
        }
        return outMessage;
    }
}
