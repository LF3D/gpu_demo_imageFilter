//
// Created by lf on 21-7-2.
//

#include "GPUImageGaussianBlurFilter.h"

const char *GPUImageGaussianBlurFilter::VERTEX_SHADER  =
        "attribute vec4 position;\n"
        "attribute vec4 inputTextureCoordinate;\n"
        "\n"
        "const int GAUSSIAN_SAMPLES = 9;\n"
        "\n"
        "uniform float texelWidthOffset;\n"
        "uniform float texelHeightOffset;\n"
        "\n"
        "varying vec2 textureCoordinate;\n"
        "varying vec2 blurCoordinates[GAUSSIAN_SAMPLES];\n"
        "\n"
        "void main()\n"
        "{\n"
        "	gl_Position = position;\n"
        "	textureCoordinate = inputTextureCoordinate.xy;\n"
        "	\n"
        "	// Calculate the positions for the blur\n"
        "	int multiplier = 0;\n"
        "	vec2 blurStep;\n"
        "   vec2 singleStepOffset = vec2(texelHeightOffset, texelWidthOffset);\n"
        "    \n"
        "	for (int i = 0; i < GAUSSIAN_SAMPLES; i++)\n"
        "   {\n"
        "		multiplier = (i - ((GAUSSIAN_SAMPLES - 1) / 2));\n"
        "       // Blur in x (horizontal)\n"
        "       blurStep = float(multiplier) * singleStepOffset;\n"
        "		blurCoordinates[i] = inputTextureCoordinate.xy + blurStep;\n"
        "	}\n"
        "}\n";

const char *GPUImageGaussianBlurFilter::FRAGMENT_SHADER =
        "uniform sampler2D inputImageTexture;\n"
        "\n"
        "const lowp int GAUSSIAN_SAMPLES = 9;\n"
        "\n"
        "varying highp vec2 textureCoordinate;\n"
        "varying highp vec2 blurCoordinates[GAUSSIAN_SAMPLES];\n"
        "\n"
        "void main()\n"
        "{\n"
        "	lowp vec3 sum = vec3(0.0);\n"
        "   lowp vec4 fragColor=texture2D(inputImageTexture,textureCoordinate);\n"
        "	\n"
        "    sum += texture2D(inputImageTexture, blurCoordinates[0]).rgb * 0.05;\n"
        "    sum += texture2D(inputImageTexture, blurCoordinates[1]).rgb * 0.09;\n"
        "    sum += texture2D(inputImageTexture, blurCoordinates[2]).rgb * 0.12;\n"
        "    sum += texture2D(inputImageTexture, blurCoordinates[3]).rgb * 0.15;\n"
        "    sum += texture2D(inputImageTexture, blurCoordinates[4]).rgb * 0.18;\n"
        "    sum += texture2D(inputImageTexture, blurCoordinates[5]).rgb * 0.15;\n"
        "    sum += texture2D(inputImageTexture, blurCoordinates[6]).rgb * 0.12;\n"
        "    sum += texture2D(inputImageTexture, blurCoordinates[7]).rgb * 0.09;\n"
        "    sum += texture2D(inputImageTexture, blurCoordinates[8]).rgb * 0.05;\n"
        "\n"
        "	gl_FragColor = vec4(sum,fragColor.a);\n"
        "}";