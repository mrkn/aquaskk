# -*- mode: ruby; coding: utf-8 -*-

require 'ConstVars'
require 'RuntimeSettings'
require 'UserDefaults'
require 'KeymapTranslator'
require 'DictionarySetTranslator'
require 'ftools'

class TransitionTask
  private

  def initialize
    @tsm_settings = RuntimeSettings.new(ConstVars::OLD_LIB_DIR,
                                        "jp.sourceforge.AquaSKKServer",
                                        "skk-jisyo.utf8",
                                        "config",
                                        "kana-rule-list")

    @imk_settings = RuntimeSettings.new(ConstVars::NEW_LIB_DIR,
                                        "jp.sourceforge.inputmethod.aquaskk",
                                        "skk-jisyo.utf8",
                                        "keymap.conf",
                                        "kana-rule.conf")

    @work_settings = @imk_settings.clone
    @work_settings.libdir = ConstVars::TEMP_DIR
    @work_settings.prefdir = ConstVars::TEMP_DIR

    @tsm_defaults = UserDefaults.new(@tsm_settings.domain)
    @imk_defaults = UserDefaults.new(@imk_settings.domain)
    @work_defaults = UserDefaults.new(@work_settings.domain)
  end

  def copy(src, dest)
    if FileTest.exist?(src) then
      File.copy(src, dest)
    end
  end

  def safe_copy(src, dest)
    if !FileTest.exist?(dest) then
      copy(src, dest)
    end
  end

  def dict_copy(src, dest)
    if !FileTest.exist?(src) then
      return
    end

    Dir.foreach(src) { |file|
      next unless file =~ /SKK-JISYO/

      safe_copy(src + '/' + file, dest + '/' + file)
    }
  end

  def task01
    copy(@tsm_defaults.path, @work_defaults.path)
    copy(@tsm_settings.config, @work_settings.config)
    copy(@tsm_settings.dictionary_set, @work_settings.dictionary_set)
  end

  def task02
    if @work_defaults.exist? then
      @work_defaults.rename('candidates_font_name', 'candidate_window_font_name')
      @work_defaults.rename('candidates_font_size', 'candidate_window_font_size')
      @work_defaults.rename('egg_like_newline', 'suppress_newline_on_commit')
      @work_defaults.rename('skkserv_enabled', 'enable_skkserv')

      @work_defaults.delete('candidates_count')
      @work_defaults.delete('candidates_window_alpha')
      @work_defaults.delete('candidates_window_color')
      @work_defaults.delete('candidates_window_show_after_nth_cand')
      @work_defaults.delete('force_ascii_mode_startup')
      @work_defaults.delete('keyboard_layout_id')
      @work_defaults.delete('numkeypad_use_halfwidth')
      @work_defaults.delete('use_brand_new_engine')
    end
  end

  def task03
    if FileTest.exist?(@work_settings.config) then
      translator = KeymapTranslator.new(@work_settings.config, @work_defaults)

      translator.execute()

      translator.save(@work_settings.config)
    end
  end

  def task04
    if FileTest.exist?(@work_settings.dictionary_set) then
      translator = DictionarySetTranslator.new(@work_settings.dictionary_set)

      translator.execute()

      translator.save(@work_settings.dictionary_set)
    end
  end

  def task05
    dict_copy(ConstVars::OLD_LIB_DIR, ConstVars::NEW_LIB_DIR)

    safe_copy(@work_defaults.path, @imk_defaults.path)
    safe_copy(@work_settings.config, @imk_settings.config)
    safe_copy(@work_settings.dictionary_set, @imk_settings.dictionary_set)

    safe_copy(@tsm_settings.user_dictionary, @imk_settings.user_dictionary)
    safe_copy(@tsm_settings.rule, @imk_settings.rule)
  end

  public

  def execute
    File.rm_f(ConstVars::TEMP_DIR)
    File.mkpath(ConstVars::TEMP_DIR)
    File.mkpath(ConstVars::NEW_LIB_DIR)

    private_methods.sort.each { |task|
      method(task).call if(task =~ /task/)
    }
  end
end
