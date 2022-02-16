package com.picovr.cloudxrclientdemo.test;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES30;
import android.opengl.GLUtils;
import android.util.Log;

import com.picovr.cloudxrclientdemo.R;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import static com.picovr.cloudxrclientdemo.test.ShaderUtil.createProgram;

public class TextureRect {
    public static float[] mMVPMatrix = new float[16];//The last effective total transformation matrix
    static float[] mMMatrix = new float[16];//The 3D transformation matrix of a specific object, including rotation, translation, and scaling
    int mProgram;//Custom rendering pipeline program id
    int muMVPMatrixHandle;//Total transformation matrix reference
    int maPositionHandle; //Vertex position attribute reference
    int maTexCoorHandle; //Vertex texture coordinate attribute reference
    String mVertexShader;//Vertex shader code script
    String mFragmentShader;//Frag shader code script
    FloatBuffer mVertexBuffer;//Vertex coordinate data buffer
    FloatBuffer mTexCoorBuffer;//Vertex texture coordinate data buffer
    int vCount = 0;

    float sRange;//s texture coordinate range
    float tRange;//t texture coordinate range

    int textureId;//Texture id assigned by the system

    public TextureRect(Context context, float sRange, float tRange) {
        this.sRange = sRange;
        this.tRange = tRange;
        init(context);
    }

    public void init(Context context) {
        initVertexData();
        initShader(context);
        initTexture(context);
    }

    //The initVertexData method to initialize vertex data
    public void initVertexData() {
        //Initialization of vertex coordinate data================begin============================
        vCount = 6;
        final float UNIT_SIZE = 1.0f;

        float[] vertices = new float[]
                {
                        -1 * UNIT_SIZE, 1 * UNIT_SIZE, 0,
                        -1 * UNIT_SIZE, -1 * UNIT_SIZE, 0,
                        1 * UNIT_SIZE, -1 * UNIT_SIZE, 0,

                        1 * UNIT_SIZE, -1 * UNIT_SIZE, 0,
                        1 * UNIT_SIZE, 1 * UNIT_SIZE, 0,
                        -1 * UNIT_SIZE, 1 * UNIT_SIZE, 0
                };

        //Create a vertex coordinate data buffer
        //vertices.length*4 is because an integer has four bytes
        ByteBuffer vbb = ByteBuffer.allocateDirect(vertices.length * 4);
        vbb.order(ByteOrder.nativeOrder());//Set byte order
        mVertexBuffer = vbb.asFloatBuffer();//Convert to Float buffer
        mVertexBuffer.put(vertices);//Put vertex coordinate data into the buffer
        mVertexBuffer.position(0);//Set the start position of the buffer
        //Special reminder: Due to the different byte order of different platforms, data units that are not bytes must pass through ByteBuffer
        //The key to conversion is to set nativeOrder() through ByteOrder, otherwise there may be problems
        //Initialization of vertex coordinate data================end============================

        //Initialization of vertex texture coordinate data================begin============================
        float[] texCoor = new float[]
                {
                        //For Neo2
                        0, 1,
                        0, 0,
                        1, 0,

                        1, 0,
                        1, 1,
                        0, 1,

                        //For Neo3
//                        0, 0, 0, 1,
//                        1, 1, 1, 1,
//                        1, 0, 0, 0,

                };
        ByteBuffer cbb = ByteBuffer.allocateDirect(texCoor.length * 4);
        cbb.order(ByteOrder.nativeOrder());//Set byte order
        mTexCoorBuffer = cbb.asFloatBuffer();//Convert to Float buffer
        mTexCoorBuffer.put(texCoor);//Put vertex coordinate data into the buffer
        mTexCoorBuffer.position(0);//Set the start position of the buffer
        //Special reminder: Due to the different byte order of different platforms, data units that are not bytes must pass through ByteBuffer
        //The key to conversion is to set nativeOrder() through ByteOrder, otherwise there may be problems
        //Initialization of vertex texture coordinate data================end============================

    }

    //Customized method of initializing the shader
    public void initShader(Context context) {
        //Load the script content of the vertex shader
        mVertexShader = com.picovr.cloudxrclientdemo.test.ShaderUtil.loadFromAssetsFile("vertex.sh", context.getResources());
        //Load the script content of the fragment shader
        mFragmentShader = com.picovr.cloudxrclientdemo.test.ShaderUtil.loadFromAssetsFile("frag.sh", context.getResources());
        //Create program based on vertex shader and fragment shader
        mProgram = createProgram(mVertexShader, mFragmentShader);
        //Get the vertex position attribute reference in the program
        maPositionHandle = GLES30.glGetAttribLocation(mProgram, "aPosition");
        //Get the reference of the vertex texture coordinate attribute in the program
        maTexCoorHandle = GLES30.glGetAttribLocation(mProgram, "aTexCoor");
        //Get the reference of the total transformation matrix in the program
        muMVPMatrixHandle = GLES30.glGetUniformLocation(mProgram, "uMVPMatrix");
    }

    public void initTexture(Context context)//textureId
    {
        //Generate texture ID
        int[] textures = new int[1];
        GLES30.glGenTextures
                (
                        1,          //Number of texture ids generated
                        textures,   //Array of texture ids
                        0           //Offset
                );
        textureId = textures[0];
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureId);
        GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_NEAREST);
        GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_SWIZZLE_R, GLES30.GL_BLUE);
        //Load image via input stream===============begin===================
        @SuppressLint("ResourceType") InputStream is = context.getResources().openRawResource(R.drawable.robot);
        Bitmap bitmapTmp;
        try {
            bitmapTmp = BitmapFactory.decodeStream(is);
        } finally {
            try {
                is.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        //Load image via input stream===============end=====================

        //Actually load textures into video memory
        GLUtils.texImage2D
                (
                        GLES30.GL_TEXTURE_2D, //Texture type
                        0,                      //The level of texture, 0 means the basic image layer, which can be understood as direct texture
                        bitmapTmp,              //Texture image
                        0                      //Texture border size
                );
        bitmapTmp.recycle();          //Release the texture map in memory after the texture is loaded successfully
    }

    public void drawSelf(int textId) {
        if (textId == -1) {
            textId = textureId;
        }
        //Specify a shader program
        GLES30.glUseProgram(mProgram);

        GLES30.glUniformMatrix4fv(muMVPMatrixHandle, 1, false, mMVPMatrix, 0);

        //Pass the vertex position data into the rendering pipeline
        GLES30.glVertexAttribPointer
                (
                        maPositionHandle,
                        3,
                        GLES30.GL_FLOAT,
                        false,
                        3 * 4,
                        mVertexBuffer
                );
        //Send texture coordinate data into the rendering pipeline
        GLES30.glVertexAttribPointer
                (
                        maTexCoorHandle,
                        2,
                        GLES30.GL_FLOAT,
                        false,
                        2 * 4,
                        mTexCoorBuffer
                );

        GLES30.glEnableVertexAttribArray(maPositionHandle);
        GLES30.glEnableVertexAttribArray(maTexCoorHandle);

        GLES30.glActiveTexture(GLES30.GL_TEXTURE0);
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textId);
        Log.d("ES30_ERROR", "glBindTexture: textId  " + textId);

        //draw
        GLES30.glDrawArrays(GLES30.GL_TRIANGLES, 0, vCount);
        GLES30.glDisableVertexAttribArray(maPositionHandle);
        GLES30.glDisableVertexAttribArray(maTexCoorHandle);
    }
}
