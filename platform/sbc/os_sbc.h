#ifndef OS_SBC_H
#define OS_SBC_H

#include "audio_driver_sbc.h"
#include "core/os/os.h"
#include "drivers/unix/os_unix.h"
#include <SDL2/SDL.h>

class OS_SBC : public OS_Unix {
	MainLoop *main_loop = nullptr;
	virtual void delete_main_loop() override;
	AudioDriverSBC audio_driver_sbc;
	bool quit_requested = false;

protected:
	virtual void initialize() override;
	virtual void initialize_joypads() override;
	virtual void set_main_loop(MainLoop *p_main_loop) override;
	virtual void finalize() override;

public:
	static OS_SBC *get_singleton();
	static OS *create();

	virtual String get_identifier() const override;
	virtual String get_name() const override;

	virtual MainLoop *get_main_loop() const override;

	virtual bool _check_internal_feature_support(const String &p_feature) override;
	void run();

	virtual String get_config_path() const override;
	virtual String get_data_path() const override;
	virtual String get_cache_path() const override;
	void set_quit_requested(bool p_quit);

	OS_SBC();
	~OS_SBC();
};

#endif // OS_SBC_H
