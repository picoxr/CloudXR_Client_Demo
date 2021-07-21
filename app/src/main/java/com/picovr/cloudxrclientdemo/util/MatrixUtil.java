package com.picovr.cloudxrclientdemo.util;

public class MatrixUtil {
    public static void quternion2Matrix(float Q[], float M[]) {
        float x = Q[0];
        float y = Q[1];
        float z = Q[2];
        float w = Q[3];
        float ww = w * w;
        float xx = x * x;
        float yy = y * y;
        float zz = z * z;

        M[0] = ww + xx - yy - zz;
        M[1] = 2 * (x * y - w * z);
        M[2] = 2 * (x * z + w * y);
        M[3] = Q[4];


        M[4] = 2 * (x * y + w * z);
        M[5] = ww - xx + yy - zz;
        M[6] = 2 * (y * z - w * x);
        M[7] = Q[5];


        M[8] = 2 * (x * z - w * y);
        ;
        M[9] = 2 * (y * z + w * x);
        M[10] = ww - xx - yy + zz;
        M[11] = Q[6];


        M[12] = 0.0f;
        M[13] = 0.0f;
        M[14] = 0.0f;
        M[15] = 1.f;
    }

    public static void matrix2Quternion(float[] m, float[] q) {

        float m00, m01, m02;
        float m10, m11, m12;
        float m20, m21, m22;

        m00 = m[0];
        m01 = m[1];
        m02 = m[2];

        m10 = m[4];
        m11 = m[5];
        m12 = m[6];

        m20 = m[8];
        m21 = m[9];
        m22 = m[10];

        float trace = m00 + m11 + m22;

        if (trace > 0.f) {
            float s = (float) (0.5f / Math.sqrt(trace + 1.0f));
            q[3] = 0.25f / s;
            q[0] = (m21 - m12) * s;
            q[1] = (m02 - m20) * s;
            q[2] = (m10 - m01) * s;
        } else {
            if (m00 > m11 && m00 > m22) {
                float s = (float) (2.0f * Math.sqrt(1.0f + m00 - m11 - m22));
                q[3] = (m21 - m12) / s;
                q[0] = 0.25f * s;
                q[1] = (m01 + m10) / s;
                q[2] = (m02 + m20) / s;
            } else if (m11 > m22) {
                float s = (float) (2.0f * Math.sqrt(1.0f + m11 - m00 - m22));
                q[3] = (m02 - m20) / s;
                q[0] = (m01 + m10) / s;
                q[1] = 0.25f * s;
                q[2] = (m12 + m21) / s;
            } else {
                float s = (float) (2.0f * Math.sqrt(1.0f + m22 - m00 - m11));
                q[3] = (m10 - m01) / s;
                q[0] = (m02 + m20) / s;
                q[1] = (m12 + m21) / s;
                q[2] = 0.25f * s;
            }
        }
    }
}
