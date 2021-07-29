/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package javaFXpackage;

import java.io.InputStream;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.stage.Stage;
import java.io.UnsupportedEncodingException;
import javafx.application.Platform;
import javafx.event.EventHandler;
import javafx.stage.WindowEvent;

/**
 *
 * @author klive
 */
public class FXMain extends Application{

    private static Stage primaryStage;
    
    @Override
    public void start(Stage primaryStage) throws Exception {
        //создание сцены
        FXMain.primaryStage = primaryStage;
        InputStream iconStream = getClass().getResourceAsStream("/javaFXpackage/resources/ClientIcon.png"); //задание пути к иконке
        Image image = new Image(iconStream);
        primaryStage.getIcons().add(image);
        primaryStage.setTitle("Client JavaFX");
        FXMLLoader loader = new FXMLLoader(getClass().getResource("/javaFXpackage/FXML.fxml"));
        Parent root = loader.load(); 
        FXMLController controller = loader.getController();
        Scene primaryScene = new Scene(root);
        primaryStage.setScene(primaryScene);
        //Остановка всех потоков при закрытии приложенгия
        primaryStage.setOnCloseRequest(new EventHandler<WindowEvent>() {
            @Override
            public void handle(WindowEvent e) {
                Platform.exit();
                System.exit(0);
            }
        });
        primaryStage.setMinHeight(350); //задание мин высоты окна
        primaryStage.setMinWidth(395); //задание макс высоты окна
        primaryStage.show(); //отображение окна
    };
    
    public static void main(String[] args) throws UnsupportedEncodingException{
        Application.launch();
    }
}