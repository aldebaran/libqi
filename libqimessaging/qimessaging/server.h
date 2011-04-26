/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef         _QI_SERVER_H_
# define        _QI_SERVER_H_

#include <qimessaging/message.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void qi_server_t;
  typedef void (*BoundMethod)(const char *complete_signature, qi_message_t *msg, qi_message_t *ret, void *data);

  qi_server_t *qi_server_create(const char *name);
  void         qi_server_destroy(qi_server_t *server);
  void         qi_server_connect(qi_server_t *server, const char *address);

  void         qi_server_advertise_service(qi_server_t *server  , const char *method_signature, BoundMethod func, void *data);
  void         qi_server_unadvertise_service(qi_server_t *server, const char *method_signature);

  /// <summary>Advertises to the master that you wish to publish data
  /// of type "PUBLISH_TYPE" to a topic of name "topicName".</summary>
  /// <param name="topicName">The name of the topic you wish to publish to.</param>
  /// <param name="isManyToMany">Allows many to many publishing</param>
  void qi_server_advertise_topic(const char *topic_name, const char *signature, const int is_many_to_many);

  /// <summary> Unadvertises a topic</summary>
  /// <param name="topicName">The name of the topic you wish to
  /// unadvertise.</param>
  void qi_server_unadvertise_topic(const char *topic_name);

  /// <summary>
  /// Publishes messages to an existing topic.
  /// e.g. publisher.publish("/time/hour_of_the_day", 10);
  /// A subscriber would be able to subscribe to the above with
  /// subscriber.subscribe("time/hour_of_the_day", &handler);
  /// where the handler expects an int type.
  /// </summary>
  /// <param name="topicName">The name of the topic you want to publish to.
  /// By convention, topic names use forward slashes as a separator
  /// e.g. "/time/hour_of_the_day"
  /// </param>
  /// <param name="publishData">The typed data that you wish to publish</param>
  /// <returns>void</returns>
  /// <seealso cref="qi::Subscriber"/>
  void qi_server_publish(const char *topic_name, const char *signature, qi_message_t *message);


#ifdef __cplusplus
}
#endif

#endif /* !_QI_H_ */
