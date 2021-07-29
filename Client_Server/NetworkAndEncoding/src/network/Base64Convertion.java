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
    
    /*������� ��� ����������� � base64*/
    private static final char[] encodingTable = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };
    
    /*�������� ������� ��� ������������� �� base64 (��� ������ decodeFromString)*/
    private static final int[] decodingTable = new int[256];
    static {
        Arrays.fill(decodingTable, -1);
        for (int i = 0; i < encodingTable.length; i++)
            decodingTable[encodingTable[i]] = i;
        decodingTable['='] = -2;
    }
    
    /**
     * ����� ����������� ������ � ������ base64
     * @param inputMessage �������� ��������� � ���� ������
     * @return encodeMessage - ������������ � base64 ��������� � ���� ������
     * @throws java.io.UnsupportedEncodingException
     */
    public static String encodeToString (String inputMessage) throws UnsupportedEncodingException{
        try{
            byte[] dataArray = inputMessage.getBytes("UTF-8"); //������� ��������� � ���� ������� ������
            int originalByteNumber; //����� �������� ������������ ��������� �������
            int appendSize; //������������ ���������� ����������� ��������
            int dataEncodeLength; // ����� ��������������� ��������� � �����
            int clearLength; //���������� "������" ���� (������� ����)
            String outMessage = new String(); //�������� ���������
            switch (dataArray.length % 3){
                case (1):
                    appendSize = 2; //����� �������� ==
                    break;
                case(2):
                    appendSize = 1; //����� �������� =
                    break;
                case(0):
                    appendSize = 0; //��� ���������
                    break;
                default:
                    appendSize = -1; //��������� ������
                    break;         
            }
            //����� ��������������� ���������
            dataEncodeLength = (dataArray.length * 8 + appendSize * 2) /  6 + appendSize;
            clearLength = (dataArray.length / 3) * 3;
            byte[] outputDataArray = new byte [dataEncodeLength]; //�������� ������ � �����������, ������������� � ������� base64
            int encodedByteNumber = 0; //����� �������� ������������� ��������� �������
            //��������� ������������ ������, �� ������ < clearLength (������ 3)
            for (originalByteNumber = 0; originalByteNumber < clearLength; originalByteNumber += 3){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //��������� ������� ����� �� ��������
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber] & 0b11) << 4) | ((dataArray[originalByteNumber + 1] >> 4) & 15)); //��������� ������� ����� �� ��������
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber + 1] & 0b1111) << 2) | ((dataArray[originalByteNumber + 2] >> 6)  & 3)); //��������� �������� ����� �� ��������
                outputDataArray[encodedByteNumber++] = (byte) (dataArray[originalByteNumber + 2] & 0b111111); //��������� ���������� ����� �� ��������
            }
            //��������� ������������ ������, �� ������ >= clearLength
            if(appendSize == 1){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //��������� ������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber] & 0b11) << 4) | ((dataArray[originalByteNumber + 1] >> 4)) & 15); //��������� ������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber + 1] & 0b1111) << 2)); //��������� �������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = 0; //��������� ���������� ����� �� ���������� ��������
            }else if(appendSize == 2){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //��������� ������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] & 0b11) << 4); //��������� ������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = 0; //��������� �������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = 0; //��������� ���������� ����� �� ���������� ��������
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
            System.err.println("�������������� ������ ��� �����������");
            return ""; 
        }
    }

    /**
     * ����� ����������� ������ � ������ ���� base64
     * @param inputMessage �������� ��������� � ���� ������
     * @return encodeMessage - ������������ � base64 ��������� � ���� ������� ����
     * @throws java.io.UnsupportedEncodingException
     */
    public static byte[] encodeToByteArray (String inputMessage) throws UnsupportedEncodingException{
        try{
            byte[] dataArray = inputMessage.getBytes("UTF-8"); //������� ��������� � ���� ������� ������
            int originalByteNumber; //����� �������� ������������ ��������� �������
            int appendSize; //������������ ���������� ����������� ��������
            int dataEncodeLength; // ����� ��������������� ��������� � �����
            int clearLength; //���������� "������" ���� (������� ����)
            switch (dataArray.length % 3){
                case (1):
                    appendSize = 2; //����� �������� ==
                    break;
                case(2):
                    appendSize = 1; //����� �������� =
                    break;
                case(0):
                    appendSize = 0; //��� ���������
                    break;
                default:
                    appendSize = -1; //��������� ������
                    break;         
            }
            //����� ��������������� ���������
            dataEncodeLength = (dataArray.length * 8 + appendSize * 2) /  6 + appendSize;
            clearLength = (dataArray.length / 3) * 3;
            byte[] outputDataArray = new byte [dataEncodeLength]; //�������� ������ � �����������, ������������� � ������� base64
            int encodedByteNumber = 0; //����� �������� ������������� ��������� �������
            //��������� ������������ ������, �� ������ < clearLength (������ 3)
            for (originalByteNumber = 0; originalByteNumber < clearLength; originalByteNumber += 3){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //��������� ������� ����� �� ��������
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber] & 0b11) << 4) | ((dataArray[originalByteNumber + 1] >> 4) & 15)); //��������� ������� ����� �� ��������
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber + 1] & 0b1111) << 2) | ((dataArray[originalByteNumber + 2] >> 6)  & 3)); //��������� �������� ����� �� ��������
                outputDataArray[encodedByteNumber++] = (byte) (dataArray[originalByteNumber + 2] & 0b111111); //��������� ���������� ����� �� ��������
            }
            //��������� ������������ ������, �� ������ >= clearLength
            if(appendSize == 1){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //��������� ������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber] & 0b11) << 4) | ((dataArray[originalByteNumber + 1] >> 4)) & 15); //��������� ������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = (byte) (((dataArray[originalByteNumber + 1] & 0b1111) << 2)); //��������� �������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = -2; //��������� ���������� ����� �� ���������� �������� (������ ������ ����� ����� -2)
            }else if(appendSize == 2){
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] >> 2) & 0b111111); //��������� ������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = (byte) ((dataArray[originalByteNumber] & 0b0011) << 4); //��������� ������� ����� �� ���������� ��������
                outputDataArray[encodedByteNumber++] = -2; //��������� �������� ����� �� ���������� �������� (������ ������ ����� ����� -2)
                outputDataArray[encodedByteNumber++] = -2; //��������� ���������� ����� �� ���������� �������� (������ ������ ����� ����� -2)
            }
            return outputDataArray;
        }catch (UnsupportedEncodingException | ArrayIndexOutOfBoundsException ex){
            System.err.println("�������������� ������ ��� �����������");
            byte[] outputDataArray = new byte [] {0};
            return outputDataArray; 
        }
    }
    
    /**
     * ����� ������������� ������ �� base64 � ������
     * @param encodeMessage ������������ � base64 ��������� � ���� ������
     * @return decodeMessage - �������������� �� base64 ��������� � ���� ������
     * @throws java.io.UnsupportedEncodingException
     */
    public static String decodeFromString (String encodeMessage) throws UnsupportedEncodingException{
        try{
            int encodeLength = encodeMessage.length();
            int countOfEqualSign = encodeMessage.endsWith("==") ? 2 : encodeMessage.endsWith("=") ? 1 : 0; //���������� ������ "�����" � �����
            int decodeArraySize = (encodeLength - countOfEqualSign) * 6 / 8; //����� ��������� ���������
            byte[] encodeArray = encodeMessage.getBytes("UTF-8");
            byte[] decodeArray = new byte[decodeArraySize];
            int encodedByteNumber; //����� ����� ��������������� ���������
            int decodedByteNumber = 0; //����� ����� ���������������� ���������
            for (encodedByteNumber = 0; encodedByteNumber < encodeLength; encodedByteNumber += 4){
                decodeArray[decodedByteNumber++] = (byte) ((decodingTable[encodeArray[encodedByteNumber]] << 2) | (decodingTable[encodeArray[encodedByteNumber + 1]] >> 4)); //������ ���� �� ������
                if (decodedByteNumber == decodeArraySize) break;
                decodeArray[decodedByteNumber++] = (byte) ((decodingTable[encodeArray[encodedByteNumber + 1]] << 4) | (decodingTable[encodeArray[encodedByteNumber + 2]] >> 2)); //������ ���� �� ������
                if (decodedByteNumber == decodeArraySize ) break;
                decodeArray[decodedByteNumber++] = (byte) ((decodingTable[encodeArray[encodedByteNumber + 2]] << 6) | (decodingTable[encodeArray[encodedByteNumber + 3]])); //������ ���� �� ������   
            }
            String decodedString = new String(decodeArray, "UTF-8");
            return decodedString;
        }catch (UnsupportedEncodingException | ArrayIndexOutOfBoundsException ex){
            System.err.println("�������������� ������ ��� �������������");
            return ""; 
        }
    }
    
    /**
     * ����� ������������� ������� ���� �� base64 � ������
     * @param encodeArray ������������ � base64 ��������� � ������� ����
     * @return decodeMessage - �������������� �� base64 ��������� � ���� ������
     * @throws java.io.UnsupportedEncodingException
     */
    public static String decodeFromByteArray (byte[] encodeArray) throws UnsupportedEncodingException{
        int encodeLength = encodeArray.length;
        int encodedByteNumber; //����� ����� ��������������� ���������
        int decodedByteNumber = 0; //����� ����� ���������������� ���������
        int decodeArraySize; //����� ��������� ���������
        int countOfEqualSign = 0; //���������� ������ "�����" � �����
        if (encodeArray[encodeLength-1] == -2){
            countOfEqualSign = 1;
            if (encodeArray[encodeLength-2] == -2){
               countOfEqualSign = 2; 
            }
        }
        decodeArraySize = (encodeLength - countOfEqualSign) * 6 / 8; //����� ��������� ���������
        byte[] decodeArray = new byte[decodeArraySize];
        for (encodedByteNumber = 0; encodedByteNumber < encodeLength; encodedByteNumber += 4){
            decodeArray[decodedByteNumber++] = (byte) ((encodeArray[encodedByteNumber] << 2) | (encodeArray[encodedByteNumber + 1] >> 4)); //������ ���� �� ������
            if (decodedByteNumber == decodeArraySize) break;
            decodeArray[decodedByteNumber++] = (byte) ((encodeArray[encodedByteNumber + 1] << 4) | (encodeArray[encodedByteNumber + 2] >> 2)); //������ ���� �� ������
            if (decodedByteNumber == decodeArraySize ) break;
            decodeArray[decodedByteNumber++] = (byte) ((encodeArray[encodedByteNumber + 2] << 6) | (encodeArray[encodedByteNumber + 3])); //������ ���� �� ������   
        }
        String decodedString = new String(decodeArray, "UTF-8");
        return decodedString;
    }
    
    /**
     * ����� �������������� ������������� ������� ���� � ������������ ������
     * @param inputArray ������������ � base64 ������ ����
     * @return outMessage - ������������ � base64 ������
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
