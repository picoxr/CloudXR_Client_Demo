package com.picovr.cloudxrclientdemo.test;

import android.content.res.Resources;
import android.opengl.GLES30;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;

public class ShaderUtil {
    //Method of creating shader program
    public static int createProgram(String vertexSource, String fragmentSource) {
        //Load vertex shader
        int vertexShader = loadShader(GLES30.GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0) {
            return 0;
        }

        //Load the frag shader
        int pixelShader = loadShader(GLES30.GL_FRAGMENT_SHADER, fragmentSource);
        if (pixelShader == 0) {
            return 0;
        }

        int program = GLES30.glCreateProgram();
        //If the program is created successfully, add vertex shader and frag shader to the program
        if (program != 0) {
            //Add a vertex shader to the program
            GLES30.glAttachShader(program, vertexShader);
            checkGlError("glAttachShader");
            //Add a frag shader to the program
            GLES30.glAttachShader(program, pixelShader);
            checkGlError("glAttachShader");
            GLES30.glLinkProgram(program);
            int[] linkStatus = new int[1];
            //Get the link status of the program
            GLES30.glGetProgramiv(program, GLES30.GL_LINK_STATUS, linkStatus, 0);
            //If the link fails, report an error and delete the program
            if (linkStatus[0] != GLES30.GL_TRUE) {
                Log.e("ES30_ERROR", "Could not link program: ");
                Log.e("ES30_ERROR", GLES30.glGetProgramInfoLog(program));
                GLES30.glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }

    //Method of loading shader
    public static int loadShader
    (
            int shaderType, //shader type  GLES30.GL_VERTEX_SHADER   GLES30.GL_FRAGMENT_SHADER
            String source   //shader script string
    ) {
        //Create a new shader
        int shader = GLES30.glCreateShader(shaderType);
        if (shader != 0) {
            //Load the source code of the shader
            GLES30.glShaderSource(shader, source);
            //Compile the shader
            GLES30.glCompileShader(shader);
            int[] compiled = new int[1];
            //Get the compilation status of Shader
            GLES30.glGetShaderiv(shader, GLES30.GL_COMPILE_STATUS, compiled, 0);
            if (compiled[0] == 0) {
                Log.e("ES30_ERROR", "Could not compile shader " + shaderType + ":");
                Log.e("ES30_ERROR", GLES30.glGetShaderInfoLog(shader));
                GLES30.glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }

    //Check every step of the operation for errors
    public static void checkGlError(String op) {
        int error;
        while ((error = GLES30.glGetError()) != GLES30.GL_NO_ERROR) {
            Log.e("ES30_ERROR", op + ": glError " + error);
            throw new RuntimeException(op + ": glError " + error);
        }
    }

    //Method of loading shader content from sh script
    public static String loadFromAssetsFile(String fname, Resources r) {
        String result = null;
        try {
            InputStream in = r.getAssets().open(fname);
            int ch = 0;
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            while ((ch = in.read()) != -1) {
                baos.write(ch);
            }
            byte[] buff = baos.toByteArray();
            baos.close();
            in.close();
            result = new String(buff, "UTF-8");
            result = result.replaceAll("\\r\\n", "\n");
        } catch (Exception e) {
            e.printStackTrace();
        }
        return result;
    }
}
