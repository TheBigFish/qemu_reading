     /* emulated ioctl list */

     IOCTL(TCGETS, IOC_R, MK_PTR(MK_STRUCT(STRUCT_termios)))
     IOCTL(TCSETS, IOC_W, MK_PTR(MK_STRUCT(STRUCT_termios)))
     IOCTL(TCSETSF, IOC_W, MK_PTR(MK_STRUCT(STRUCT_termios)))
     IOCTL(TCSETSW, IOC_W, MK_PTR(MK_STRUCT(STRUCT_termios)))
     IOCTL(TIOCGWINSZ, IOC_R, MK_PTR(MK_STRUCT(STRUCT_winsize)))
     IOCTL(TIOCSWINSZ, IOC_W, MK_PTR(MK_STRUCT(STRUCT_winsize)))
     IOCTL(FIONREAD, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TCGETA, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TCSETA, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TCSETAW, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TCSETAF, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TCSBRK, 0, TYPE_INT)
     IOCTL(TCSBRKP, 0, TYPE_INT)
     IOCTL(TCXONC, 0, TYPE_INT)
     IOCTL(TCFLSH, 0, TYPE_INT)
     IOCTL(TIOCEXCL, 0, TYPE_NULL)
     IOCTL(TIOCNXCL, 0, TYPE_NULL)
     IOCTL(TIOCSCTTY, 0, TYPE_INT)
     IOCTL(TIOCGPGRP, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TIOCSPGRP, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCOUTQ, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TIOCSTI, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCMGET, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TIOCMBIS, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCMBIC, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCMSET, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCGSOFTCAR, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TIOCSSOFTCAR, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCLINUX, IOC_R | IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCCONS, 0, TYPE_NULL)
     IOCTL(TIOCGSERIAL, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TIOCSSERIAL, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCPKT, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(FIONBIO, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCNOTTY, 0, TYPE_NULL)
     IOCTL(TIOCGETD, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TIOCSETD, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCGPTN, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TIOCSPTLCK, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(FIOCLEX, 0, TYPE_NULL)
     IOCTL(FIONCLEX, 0, TYPE_NULL)
     IOCTL(FIOASYNC, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(TIOCGLCKTRMIOS, IOC_R, MK_PTR(MK_STRUCT(STRUCT_termios)))
     IOCTL(TIOCSLCKTRMIOS, IOC_W, MK_PTR(MK_STRUCT(STRUCT_termios)))
     IOCTL(TIOCSERCONFIG, 0, TYPE_NULL)
     IOCTL(TIOCSERGETLSR, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(TIOCSERGETMULTI, IOC_R, MK_PTR(MK_STRUCT(STRUCT_serial_multiport_struct)))
     IOCTL(TIOCSERSETMULTI, IOC_W, MK_PTR(MK_STRUCT(STRUCT_serial_multiport_struct)))
     IOCTL(TIOCMIWAIT, 0, TYPE_INT)
     IOCTL(TIOCGICOUNT, IOC_R, MK_PTR(MK_STRUCT(STRUCT_serial_icounter_struct)))

     IOCTL(BLKROSET, IOC_W, MK_PTR(TYPE_INT))
     IOCTL(BLKROGET, IOC_R, MK_PTR(TYPE_INT))
     IOCTL(BLKRRPART, 0, TYPE_NULL)
     IOCTL(BLKGETSIZE, IOC_R, MK_PTR(TYPE_ULONG))
#ifdef BLKGETSIZE64
     IOCTL(BLKGETSIZE64, IOC_R, MK_PTR(TYPE_ULONGLONG))
#endif
     IOCTL(BLKFLSBUF, 0, TYPE_NULL)
     IOCTL(BLKRASET, 0, TYPE_INT)
     IOCTL(BLKRAGET, IOC_R, MK_PTR(TYPE_LONG))
#ifdef FIBMAP
     IOCTL(FIBMAP, IOC_W | IOC_R, MK_PTR(TYPE_LONG))
#endif
#ifdef FIGETBSZ
     IOCTL(FIGETBSZ, IOC_R, MK_PTR(TYPE_LONG))
#endif

  IOCTL(SIOCATMARK, 0, TYPE_NULL)
  IOCTL(SIOCADDRT, IOC_W, MK_PTR(MK_STRUCT(STRUCT_rtentry)))
  IOCTL(SIOCDELRT, IOC_W, MK_PTR(MK_STRUCT(STRUCT_rtentry)))
  IOCTL(SIOCGIFNAME, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SIOCGIFFLAGS, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_short_ifreq)))
  IOCTL(SIOCSIFFLAGS, IOC_W, MK_PTR(MK_STRUCT(STRUCT_short_ifreq)))
  IOCTL(SIOCGIFADDR, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCSIFADDR, IOC_W, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCGIFBRDADDR, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCSIFBRDADDR, IOC_W, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCGIFDSTADDR, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCSIFDSTADDR, IOC_W, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCGIFNETMASK, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCSIFNETMASK, IOC_W, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCGIFHWADDR, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCSIFHWADDR, IOC_W, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCGIFTXQLEN, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCSIFTXQLEN, IOC_W, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCGIFMETRIC, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_int_ifreq)))
  IOCTL(SIOCSIFMETRIC, IOC_W, MK_PTR(MK_STRUCT(STRUCT_int_ifreq)))
  IOCTL(SIOCGIFMTU, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_int_ifreq)))
  IOCTL(SIOCSIFMTU, IOC_W, MK_PTR(MK_STRUCT(STRUCT_int_ifreq)))
  IOCTL(SIOCGIFMAP, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_ifmap_ifreq)))
  IOCTL(SIOCSIFMAP, IOC_W, MK_PTR(MK_STRUCT(STRUCT_ifmap_ifreq)))
  IOCTL(SIOCGIFSLAVE, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_char_ifreq)))
  IOCTL(SIOCSIFSLAVE, IOC_W, MK_PTR(MK_STRUCT(STRUCT_char_ifreq)))
  IOCTL(SIOCGIFMEM, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_ptr_ifreq)))
  IOCTL(SIOCSIFMEM, IOC_W, MK_PTR(MK_STRUCT(STRUCT_ptr_ifreq)))
  IOCTL(SIOCADDMULTI, IOC_W, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCDELMULTI, IOC_W, MK_PTR(MK_STRUCT(STRUCT_sockaddr_ifreq)))
  IOCTL(SIOCSIFLINK, 0, TYPE_NULL)
  IOCTL(SIOCGIFCONF, IOC_W | IOC_R, MK_PTR(MK_STRUCT(STRUCT_ifconf)))
  IOCTL(SIOCGIFENCAP, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SIOCSIFENCAP, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SIOCDARP, IOC_W, MK_PTR(MK_STRUCT(STRUCT_arpreq)))
  IOCTL(SIOCSARP, IOC_W, MK_PTR(MK_STRUCT(STRUCT_arpreq)))
  IOCTL(SIOCGARP, IOC_R, MK_PTR(MK_STRUCT(STRUCT_arpreq)))
  IOCTL(SIOCDRARP, IOC_W, MK_PTR(MK_STRUCT(STRUCT_arpreq)))
  IOCTL(SIOCSRARP, IOC_W, MK_PTR(MK_STRUCT(STRUCT_arpreq)))
  IOCTL(SIOCGRARP, IOC_R, MK_PTR(MK_STRUCT(STRUCT_arpreq)))

  IOCTL(CDROMPAUSE, 0, TYPE_NULL)
  IOCTL(CDROMSTART, 0, TYPE_NULL)
  IOCTL(CDROMSTOP, 0, TYPE_NULL)
  IOCTL(CDROMRESUME, 0, TYPE_NULL)
  IOCTL(CDROMEJECT, 0, TYPE_NULL)
  IOCTL(CDROMEJECT_SW, 0, TYPE_INT)
  IOCTL(CDROMCLOSETRAY, 0, TYPE_NULL)
  IOCTL(CDROMRESET, 0, TYPE_NULL)
  IOCTL(CDROMPLAYMSF, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(CDROMPLAYTRKIND, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(CDROMREADTOCHDR, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(CDROMREADTOCENTRY, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(CDROMVOLCTRL, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(CDROMSUBCHNL, IOC_RW, MK_PTR(TYPE_INT))
  /* XXX: incorrect (need specific handling) */
  IOCTL(CDROMREADAUDIO, IOC_W, MK_PTR(MK_STRUCT(STRUCT_cdrom_read_audio)))
  IOCTL(CDROMREADCOOKED, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(CDROMREADRAW, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(CDROMREADMODE1, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(CDROMREADMODE2, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(CDROMREADALL, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(CDROMMULTISESSION, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(CDROM_GET_UPC, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(CDROMVOLREAD, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(CDROMSEEK, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(CDROMPLAYBLK, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(CDROM_MEDIA_CHANGED, 0, TYPE_NULL)
  IOCTL(CDROM_SET_OPTIONS, 0, TYPE_INT)
  IOCTL(CDROM_CLEAR_OPTIONS, 0, TYPE_INT)
  IOCTL(CDROM_SELECT_SPEED, 0, TYPE_INT)
  IOCTL(CDROM_SELECT_DISC, 0, TYPE_INT)
  IOCTL(CDROM_DRIVE_STATUS, 0, TYPE_NULL)
  IOCTL(CDROM_DISC_STATUS, 0, TYPE_NULL)
  IOCTL(CDROMAUDIOBUFSIZ, 0, TYPE_INT)

  IOCTL(SNDCTL_COPR_HALT, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_COPR_LOAD, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_COPR_RCODE, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_COPR_RCVMSG, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_COPR_RDATA, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_COPR_RESET, 0, TYPE_NULL)
  IOCTL(SNDCTL_COPR_RUN, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_COPR_SENDMSG, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_COPR_WCODE, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_COPR_WDATA, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_CHANNELS, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_GETBLKSIZE, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_GETCAPS, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_GETFMTS, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_GETIPTR, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_GETISPACE, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_GETOPTR, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_GETOSPACE, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_GETTRIGGER, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_MAPINBUF, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_MAPOUTBUF, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_NONBLOCK, 0, TYPE_NULL)
  IOCTL(SNDCTL_DSP_POST, 0, TYPE_NULL)
  IOCTL(SNDCTL_DSP_RESET, 0, TYPE_NULL)
  IOCTL(SNDCTL_DSP_SETDUPLEX, 0, TYPE_NULL)
  IOCTL(SNDCTL_DSP_SETFMT, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_SETFRAGMENT, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_SETSYNCRO, 0, TYPE_NULL)
  IOCTL(SNDCTL_DSP_SETTRIGGER, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_SPEED, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_STEREO, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_SUBDIVIDE, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_DSP_SYNC, 0, TYPE_NULL)
  IOCTL(SNDCTL_FM_4OP_ENABLE, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_FM_LOAD_INSTR, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_MIDI_INFO, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_MIDI_MPUCMD, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_MIDI_MPUMODE, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_MIDI_PRETIME, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_CTRLRATE, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_GETINCOUNT, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_GETOUTCOUNT, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_NRMIDIS, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_NRSYNTHS, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_OUTOFBAND, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_PANIC, 0, TYPE_NULL)
  IOCTL(SNDCTL_SEQ_PERCMODE, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_RESET, 0, TYPE_NULL)
  IOCTL(SNDCTL_SEQ_RESETSAMPLES, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_SYNC, 0, TYPE_NULL)
  IOCTL(SNDCTL_SEQ_TESTMIDI, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SEQ_THRESHOLD, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SYNTH_INFO, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_SYNTH_MEMAVL, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_TMR_CONTINUE, 0, TYPE_NULL)
  IOCTL(SNDCTL_TMR_METRONOME, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_TMR_SELECT, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_TMR_SOURCE, IOC_RW, MK_PTR(TYPE_INT))
#if 0
     /* we invalidate these defines because they have a same number as
        termios ioctls */
  IOCTL(SNDCTL_TMR_START, 0, TYPE_NULL)
  IOCTL(SNDCTL_TMR_STOP, 0, TYPE_NULL)
#endif
  IOCTL(SNDCTL_TMR_TEMPO, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SNDCTL_TMR_TIMEBASE, IOC_RW, MK_PTR(TYPE_INT))

  IOCTL(SOUND_PCM_WRITE_FILTER, IOC_W | IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_PCM_READ_RATE, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_PCM_READ_CHANNELS, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_PCM_READ_BITS, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_PCM_READ_FILTER, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_INFO, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_ACCESS, 0, TYPE_PTRVOID)
  IOCTL(SOUND_MIXER_PRIVATE1, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_PRIVATE2, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_PRIVATE3, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_PRIVATE4, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_PRIVATE5, IOC_RW, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_VOLUME, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_BASS, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_TREBLE, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_SYNTH, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_PCM, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_SPEAKER, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_LINE, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_MIC, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_CD, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_IMIX, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_ALTPCM, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_RECLEV, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_IGAIN, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_OGAIN, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_LINE1, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_LINE2, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_LINE3, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_MUTE, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_ENHANCE, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_LOUD, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_RECSRC, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_DEVMASK, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_RECMASK, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_STEREODEVS, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_READ_CAPS, IOC_R, MK_PTR(TYPE_INT))

  IOCTL(SOUND_MIXER_WRITE_VOLUME, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_BASS, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_TREBLE, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_SYNTH, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_PCM, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_SPEAKER, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_LINE, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_MIC, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_CD, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_IMIX, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_ALTPCM, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_RECLEV, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_IGAIN, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_OGAIN, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_LINE1, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_LINE2, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_LINE3, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_MUTE, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_ENHANCE, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_LOUD, IOC_W, MK_PTR(TYPE_INT))
  IOCTL(SOUND_MIXER_WRITE_RECSRC, IOC_W, MK_PTR(TYPE_INT))

  IOCTL(HDIO_GETGEO, IOC_R, MK_PTR(MK_STRUCT(STRUCT_hd_geometry)))
  IOCTL(HDIO_GET_UNMASKINTR, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(HDIO_GET_MULTCOUNT, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(HDIO_GET_IDENTITY, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(HDIO_GET_KEEPSETTINGS, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(HDIO_GET_NOWERR, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(HDIO_GET_DMA, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(HDIO_GET_32BIT, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(HDIO_DRIVE_CMD, IOC_R, MK_PTR(TYPE_INT))
  IOCTL(HDIO_SET_UNMASKINTR, 0, TYPE_INT)
  IOCTL(HDIO_SET_MULTCOUNT, 0, TYPE_INT)
  IOCTL(HDIO_SET_KEEPSETTINGS, 0, TYPE_INT)
  IOCTL(HDIO_SET_NOWERR, 0, TYPE_INT)
  IOCTL(HDIO_SET_DMA, 0, TYPE_INT)
  IOCTL(HDIO_SET_32BIT, 0, TYPE_INT)
  IOCTL(HDIO_SET_PIO_MODE, 0, TYPE_INT)

  IOCTL(VFAT_IOCTL_READDIR_BOTH, IOC_R, MK_PTR(MK_ARRAY(MK_STRUCT(STRUCT_dirent), 2)))
  IOCTL(VFAT_IOCTL_READDIR_SHORT, IOC_R, MK_PTR(MK_ARRAY(MK_STRUCT(STRUCT_dirent), 2)))
