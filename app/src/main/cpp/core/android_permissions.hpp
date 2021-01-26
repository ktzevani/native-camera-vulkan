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

#ifndef NCV_ANDROID_PERMISSIONS_HPP
#define NCV_ANDROID_PERMISSIONS_HPP

class android_app;

namespace core
{
    class android_permissions
    {
    public:
        static bool is_camera_permitted(android_app *a_app);
        static void request_camera_permission(android_app *a_app);
    };
}

#endif //NCV_ANDROID_PERMISSIONS_HPP
