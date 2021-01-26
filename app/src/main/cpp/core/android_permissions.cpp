/*
 * Copyright 2020 Konstantinos Tzevanidis
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <core/android_permissions.hpp>

#include <android_native_app_glue.h>
#include <stdexcept>
#include <string>

using namespace ::std;

static jclass load_java_class(JNIEnv* a_env, jobject a_instance, const string& a_class_name)
{
    jclass NativeActivity = a_env->FindClass("android/app/NativeActivity");
    jclass ClassLoader = a_env->FindClass("java/lang/ClassLoader");

    jmethodID NativeActivity_getClassLoader =
            a_env->GetMethodID(NativeActivity, "getClassLoader", "()Ljava/lang/ClassLoader;");

    jobject ClassLoaderObj = a_env->CallObjectMethod(a_instance, NativeActivity_getClassLoader);

    jmethodID loadClass =
            a_env->GetMethodID(ClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    jstring j_class_name = a_env->NewStringUTF(a_class_name.c_str());

    return static_cast<jclass>(a_env->CallObjectMethod(ClassLoaderObj, loadClass, j_class_name));
}

namespace core
{
    void android_permissions::request_camera_permission(android_app *a_app)
    {
        auto instance = a_app->activity->clazz;
        auto env = a_app->activity->env;
        auto vm = a_app->activity->vm;

        if(vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
            throw runtime_error("Could not attach thread to running VM.");

        jclass ActivityCompat = load_java_class(env, instance, "androidx/core/app/ActivityCompat");

        jmethodID ActivityCompat_requestPermissions =
            env->GetStaticMethodID(ActivityCompat, "requestPermissions",
                   "(Landroid/app/Activity;[Ljava/lang/String;I)V");

        jstring camera_permission = env->NewStringUTF("android.permission.CAMERA");
        jobjectArray perms = env->NewObjectArray(1, env->FindClass("java/lang/String"), nullptr);
        env->SetObjectArrayElement(perms, 0, camera_permission);
        env->CallStaticVoidMethod(ActivityCompat, ActivityCompat_requestPermissions, instance, perms, 1);

        vm->DetachCurrentThread();
    }

    bool android_permissions::is_camera_permitted(android_app *a_app)
    {
        auto instance = a_app->activity->clazz;
        auto env = a_app->activity->env;
        auto vm = a_app->activity->vm;

        if(vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
            throw runtime_error("Could not attach thread to running VM.");

        jclass ContextCompat = load_java_class(env, instance, "androidx/core/content/ContextCompat");

        jmethodID ContextCompat_checkSelfPermission =
            env->GetStaticMethodID(ContextCompat, "checkSelfPermission",
               "(Landroid/content/Context;Ljava/lang/String;)I");

        jstring camera_permission = env->NewStringUTF("android.permission.CAMERA");
        jint result = env->CallStaticIntMethod(ContextCompat, ContextCompat_checkSelfPermission,
                                               instance, camera_permission);

        vm->DetachCurrentThread();

        return result == 0;
    }
}