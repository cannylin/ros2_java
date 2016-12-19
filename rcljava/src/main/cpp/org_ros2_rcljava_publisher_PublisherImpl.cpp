// Copyright 2016 Esteve Fernandez <esteve@apache.org>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <jni.h>

#include <string>
#include <cstdlib>
#include <cassert>
#include <cstdio>

#include "rmw/rmw.h"
#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rcl/node.h"

#include "rcljava_common/exceptions.h"
#include "rcljava_common/signatures.h"

#include "org_ros2_rcljava_publisher_PublisherImpl.h"

JNIEXPORT void JNICALL Java_org_ros2_rcljava_publisher_PublisherImpl_nativePublish(JNIEnv * env,
  jclass, jlong publisher_handle, jobject jmsg)
{
  rcl_publisher_t * publisher = reinterpret_cast<rcl_publisher_t *>(publisher_handle);

  jclass jmessage_class = env->GetObjectClass(jmsg);

  jmethodID mid = env->GetStaticMethodID(jmessage_class, "getFromJavaConverter", "()J");
  jlong jfrom_java_converter = env->CallStaticLongMethod(jmessage_class, mid);

  convert_from_java_signature convert_from_java =
    reinterpret_cast<convert_from_java_signature>(jfrom_java_converter);

  void * raw_ros_message = convert_from_java(jmsg, nullptr);

  rcl_ret_t ret = rcl_publish(publisher, raw_ros_message);
  if (ret != RCL_RET_OK) {
    rcljava_throw_exception(
      env, "java/lang/IllegalStateException",
      "Failed to publish: " + std::string(rcl_get_error_string_safe()));
  }
}

JNIEXPORT void JNICALL Java_org_ros2_rcljava_publisher_PublisherImpl_nativeDispose(JNIEnv * env,
  jclass, jlong node_handle, jlong publisher_handle)
{
  if (publisher_handle == 0) {
    // everything is ok, already destroyed
    return;
  }

  if (node_handle == 0) {
    // TODO(esteve): handle this, node is null, but publisher isn't
    return;
  }

  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(node_handle);

  rcl_publisher_t * publisher = reinterpret_cast<rcl_publisher_t *>(publisher_handle);

  assert(publisher != NULL);

  rcl_ret_t ret = rcl_publisher_fini(publisher, node);

  if (ret != RCL_RET_OK) {
    rcljava_throw_exception(
      env, "java/lang/IllegalStateException",
      "Failed to destroy publisher: " + std::string(rcl_get_error_string_safe()));
  }
}