/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_TRANSLATOR_HPP_
#define _QI_TRANSLATOR_HPP_

# include <boost/noncopyable.hpp>
# include <string>
# include <qi/api.hpp>


namespace qi
{
  class TranslatorPrivate;
  /**
   * \brief Localization of your source code
   *
   *        make your application or library speak in the user's language.
   * \includename{qi/translator.hpp}
   */
  class QI_API Translator : private boost::noncopyable
  {
  public:
    /**
     * \brief Constructor.
     * \param name Application or Library name
     */
    Translator(const std::string &name);
    ~Translator();

    /**
     * \brief Translate a message.
     *
     * \param msg     Message to translate
     * \param domain  Domain name
     * \param locale  Locale name
     * \param context Context of the msg
     * \return The translated message
     */
    std::string translate(const std::string &msg,
                          const std::string &domain = "",
                          const std::string &locale = "",
                          const std::string &context = "");

    /**
     * \brief Translate a message with a specific context.
     *
     * \param msg     Message to translate
     * \param context Context of the msg
     * \return The translated message
     */
    std::string translateContext(const std::string &msg,
                                 const std::string &context);

    /**
     * \brief Change the locale at runtime.
     *
     * \param locale Locale name formatted as xx_XX (country and language code).
     */
    void setCurrentLocale(const std::string &locale);
    /**
     * \brief Set the default Domain.
     *
     * \param domain Domain name.
     */
    void setDefaultDomain(const std::string &domain);
    /**
     * \brief Add a new dicationary of messages.
     *
     * \param domain Domain name.
     */
    void addDomain(const std::string &domain);

  private:
    TranslatorPrivate *_p;
  };

  /**
   * \brief Get a reference on the default Translator.
   *
   * \param name Application or Library name
   * \return A reference on default qi::Translator
   */
  QI_API qi::Translator &defaultTranslator(const std::string &name);

  /**
   * \copydoc qi::Translator::translate
   */
  QI_API std::string tr(const std::string &msg,
                        const std::string &domain = "",
                        const std::string &locale = "",
                        const std::string &context = "");

  /**
   * \copydoc qi::Translator::translateContext
   */
  QI_API std::string trContext(const std::string &msg,
                               const std::string &context);
  namespace detail
  {
    QI_API void addDomainPath(const std::string &path);
    QI_API void removeDomainPath(const std::string &path);
  }

}

#endif  // _QI_TRANSLATOR_HPP_
