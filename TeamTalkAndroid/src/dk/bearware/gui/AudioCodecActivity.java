package dk.bearware.gui;

import java.util.Locale;

import dk.bearware.AudioCodec;
import dk.bearware.Codec;
import dk.bearware.OpusCodec;
import dk.bearware.OpusConstants;
import dk.bearware.SpeexCodec;
import dk.bearware.SpeexConstants;
import dk.bearware.SpeexVBRCodec;
import dk.bearware.data.MapAdapter;

import android.app.Activity;
import android.app.ActionBar;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.support.v13.app.FragmentPagerAdapter;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

public class AudioCodecActivity extends Activity implements
    ActionBar.TabListener {

    static final int TAB_OPUS       = 0,
                     TAB_SPEEX      = 1,
                     TAB_SPEEXVBR   = 2,
                     TAB_NOAUDIO    = 3,
                     
                     TAB_COUNT      = 4;
    /**
     * The {@link android.support.v4.view.PagerAdapter} that will provide fragments for each of the sections. We use a
     * {@link FragmentPagerAdapter} derivative, which will keep every loaded fragment in memory. If this becomes too
     * memory intensive, it may be best to switch to a {@link android.support.v13.app.FragmentStatePagerAdapter}.
     */
    SectionsPagerAdapter mSectionsPagerAdapter;

    /**
     * The {@link ViewPager} that will host the section contents.
     */
    ViewPager mViewPager;
    
    AudioCodec audiocodec;
    
    OpusCodec opuscodec = new OpusCodec(true);
    SpeexCodec speexcodec = new SpeexCodec(true);
    SpeexVBRCodec speexvbrcodec = new SpeexVBRCodec(true);
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audiocodec);

        audiocodec = Utils.getAudioCodec(this.getIntent());

        int tab_index = 0;
        switch(audiocodec.nCodec) {
            case Codec.OPUS_CODEC :
                opuscodec = audiocodec.opus;
                tab_index = TAB_OPUS;
                break;
            case Codec.SPEEX_CODEC :
                speexcodec = audiocodec.speex;
                tab_index = TAB_SPEEX;
                break;
            case Codec.SPEEX_VBR_CODEC :
                speexvbrcodec = audiocodec.speex_vbr;
                tab_index = TAB_SPEEXVBR;
                break;
            case Codec.NO_CODEC :
                tab_index = TAB_NOAUDIO;
                break;
        }
        
        // Set up the action bar.
        final ActionBar actionBar = getActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
        actionBar.setDisplayHomeAsUpEnabled(true);
        
        // Create the adapter that will return a fragment for each of the three
        // primary sections of the activity.
        mSectionsPagerAdapter = new SectionsPagerAdapter(getFragmentManager());

        // Set up the ViewPager with the sections adapter.
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);

        // When swiping between different sections, select the corresponding
        // tab. We can also use ActionBar.Tab#select() to do this if we have
        // a reference to the Tab.
        mViewPager.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                actionBar.setSelectedNavigationItem(position);
            }
        });

        // For each of the sections in the app, add a tab to the action bar.
        for(int i = 0;i < mSectionsPagerAdapter.getCount();i++) {
            // Create a tab with text corresponding to the page title defined by
            // the adapter. Also specify this Activity object, which implements
            // the TabListener interface, as the callback (listener) for when
            // this tab is selected.
            actionBar.addTab(actionBar.newTab().setText(
                mSectionsPagerAdapter.getPageTitle(i)).setTabListener(this));
        }
        
        mViewPager.setCurrentItem(tab_index);
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home : {
                int i = mViewPager.getCurrentItem();
                Fragment frag = mSectionsPagerAdapter.getItem(i);
                switch (i) {
                    case TAB_OPUS : {
                        OPUSFragment opusfrag = (OPUSFragment)frag;
                        opusfrag.exchangeOpusCodec(opusfrag.getView(), true);
                        audiocodec.nCodec = Codec.OPUS_CODEC;
                        audiocodec.opus = opuscodec;
                        break;
                    }
                    case TAB_SPEEX : {
                        SpeexFragment spxfrag = (SpeexFragment)frag;
                        spxfrag.exchangeSpeexCodec(spxfrag.getView(), true);
                        audiocodec.nCodec = Codec.SPEEX_CODEC;
                        audiocodec.speex = speexcodec;
                        break;
                    }
                    case TAB_SPEEXVBR : {
                        SpeexVBRFragment spxfrag = (SpeexVBRFragment)frag;
                        spxfrag.exchangeSpeexVBRCodec(spxfrag.getView(), true);
                        audiocodec.nCodec = Codec.SPEEX_VBR_CODEC;
                        audiocodec.speex_vbr = speexvbrcodec;
                        break;
                    }
                    case TAB_NOAUDIO : {
                        audiocodec.nCodec = Codec.NO_CODEC;
                        break;
                    }
                }

                Intent intent = getIntent();
                setResult(RESULT_OK, Utils.putAudioCodec(intent, audiocodec)); 
                finish();
                break;
            }
        }
        
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onTabSelected(ActionBar.Tab tab,
        FragmentTransaction fragmentTransaction) {
        // When the given tab is selected, switch to the corresponding page in
        // the ViewPager.
        mViewPager.setCurrentItem(tab.getPosition());
    }

    @Override
    public void onTabUnselected(ActionBar.Tab tab,
        FragmentTransaction fragmentTransaction) {
    }

    @Override
    public void onTabReselected(ActionBar.Tab tab,
        FragmentTransaction fragmentTransaction) {
    }
    
    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to one of the sections/tabs/pages.
     */
    public class SectionsPagerAdapter extends FragmentPagerAdapter {

        public SectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        OPUSFragment opusfrag;
        SpeexFragment speexfrag;
        SpeexVBRFragment speexvbrfrag;
        NoAudioFragment noaudiofrag;
        
        @Override
        public Fragment getItem(int position) {
            switch (position) {
                default :
                case TAB_OPUS :
                    if(opusfrag == null)
                        opusfrag = new OPUSFragment(opuscodec);
                    return opusfrag;
                case TAB_SPEEX :
                    if(speexfrag == null)
                        speexfrag = new SpeexFragment(speexcodec);
                    return speexfrag;
                case TAB_SPEEXVBR :
                    if(speexvbrfrag == null)
                        speexvbrfrag = new SpeexVBRFragment(speexvbrcodec);
                    return speexvbrfrag;
                case TAB_NOAUDIO :
                    if(noaudiofrag == null)
                        noaudiofrag = new NoAudioFragment();
                    return noaudiofrag;
            }
        }

        @Override
        public int getCount() {
            return TAB_COUNT;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            Locale l = Locale.getDefault();
            switch(position) {
                case TAB_OPUS :
                    return getString(R.string.title_section_opus).toUpperCase(l);
                case TAB_SPEEX :
                    return getString(R.string.title_section_speex).toUpperCase(l);
                case TAB_SPEEXVBR :
                    return getString(R.string.title_section_speexvbr).toUpperCase(l);
                case TAB_NOAUDIO :
                    return getString(R.string.title_section_noaudio).toUpperCase(l);
            }
            return null;
        }
    }
    
    /**
     * A placeholder fragment for OPUS.
     */
    public static class OPUSFragment extends Fragment {

        OpusCodec opuscodec;
        
        MapAdapter appMap;
        MapAdapter srMap;
        MapAdapter audMap;
        MapAdapter delayMap;
        
        public OPUSFragment(OpusCodec opuscodec) {
            this.opuscodec = opuscodec;
        }
        
        @Override
        public void onAttach(Activity activity) {
            
            appMap = new MapAdapter(activity);
            srMap = new MapAdapter(activity);
            audMap = new MapAdapter(activity);
            delayMap = new MapAdapter(activity);

            appMap.addPair("VoIP", OpusConstants.OPUS_APPLICATION_VOIP);
            appMap.addPair("Music", OpusConstants.OPUS_APPLICATION_AUDIO);

            srMap.addPair("8 KHz", 8000);
            srMap.addPair("12 KHz", 12000);
            srMap.addPair("16 KHz", 16000);
            srMap.addPair("24 KHz", 24000);
            srMap.addPair("48 KHz", 48000);

            audMap.addPair("Mono", 1);
            audMap.addPair("Stereo", 2);

            delayMap.addPair("20 msec", 20);
            delayMap.addPair("40 msec", 40);
            delayMap.addPair("60 msec", 60);

            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_audiocodec_opus,
                                        container, false);
            
            exchangeOpusCodec(rootView, false);
            
            return rootView;
        }
        
        void exchangeOpusCodec(View rootView, boolean store) {
            
            Spinner app = (Spinner)rootView.findViewById(R.id.opus_appSpin);
            Spinner sr = (Spinner)rootView.findViewById(R.id.opus_samplerateSpin);
            Spinner audchan = (Spinner)rootView.findViewById(R.id.opus_audchanSpin);
            CheckBox dtx = (CheckBox)rootView.findViewById(R.id.opus_dtxCheckBox);
            SeekBar bitrate = (SeekBar)rootView.findViewById(R.id.opus_bitrateSeekBar);
            final TextView bitrateText = (TextView)rootView.findViewById(R.id.opus_brTextView);
            Spinner delay = (Spinner)rootView.findViewById(R.id.opus_txmsecSpin);

            if(store) {
                opuscodec.nApplication = appMap.getValue(app.getSelectedItemPosition(), 
                                                         OpusConstants.DEFAULT_OPUS_APPLICATION);
                opuscodec.nSampleRate = srMap.getValue(sr.getSelectedItemPosition(), 
                                                       OpusConstants.DEFAULT_OPUS_SAMPLERATE);
                opuscodec.nChannels = audMap.getValue(audchan.getSelectedItemPosition(),
                                                      OpusConstants.DEFAULT_OPUS_CHANNELS);
                opuscodec.nComplexity = OpusConstants.DEFAULT_OPUS_COMPLEXITY;
                opuscodec.bFEC = OpusConstants.DEFAULT_OPUS_FEC;
                opuscodec.bDTX = dtx.isChecked(); 
                opuscodec.bVBR = OpusConstants.DEFAULT_OPUS_VBR;
                opuscodec.bVBRConstraint = OpusConstants.DEFAULT_OPUS_VBRCONSTRAINT;
                opuscodec.nBitRate = bitrate.getProgress() * 1000 + OpusConstants.OPUS_MIN_BITRATE; 
                opuscodec.nMSecPerPacket = delayMap.getValue(delay.getSelectedItemPosition(),
                                                             OpusConstants.DEFAULT_OPUS_DELAY);
            }
            else {
                app.setAdapter(appMap);
                app.setSelection(appMap.getIndex(opuscodec.nApplication, 0));

                sr.setAdapter(srMap);
                sr.setSelection(srMap.getIndex(opuscodec.nSampleRate, 0));
                
                dtx.setChecked(opuscodec.bDTX);
                
                audchan.setAdapter(audMap);
                audchan.setSelection(audMap.getIndex(opuscodec.nChannels, 0));
                
                int max_br = OpusConstants.OPUS_MAX_BITRATE - OpusConstants.OPUS_MIN_BITRATE;
                max_br /= 1000;
                bitrate.setMax(max_br);
                
                bitrate.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    
                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                    }
                    
                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }
                    
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress,
                                                  boolean fromUser) {
                        int br = progress + (OpusConstants.OPUS_MIN_BITRATE / 1000);
                        bitrateText.setText(br + " kbit/s");
                    }
                });

                bitrate.setProgress((opuscodec.nBitRate / 1000) + 
                                    (OpusConstants.OPUS_MIN_BITRATE / 1000)); 

                delay.setAdapter(delayMap);
                delay.setSelection(delayMap.getIndex(opuscodec.nMSecPerPacket, 0));
            }
        }
    }
    
    public static class SpeexFragment extends Fragment {

        SpeexCodec speexcodec;
        
        MapAdapter srMap;
        MapAdapter delayMap;
        
        public SpeexFragment(SpeexCodec speexcodec) {
            this.speexcodec = speexcodec;
        }

        @Override
        public void onAttach(Activity activity) {
            
            srMap = new MapAdapter(activity);
            srMap.addPair("8 KHz", SpeexConstants.SPEEX_BANDMODE_NARROW);
            srMap.addPair("16 KHz", SpeexConstants.SPEEX_BANDMODE_WIDE);
            srMap.addPair("32 KHz", SpeexConstants.SPEEX_BANDMODE_UWIDE);
            
            delayMap = new MapAdapter(activity);
            delayMap.addPair("20 msec", 20);
            delayMap.addPair("40 msec", 40);
            delayMap.addPair("60 msec", 60);
            delayMap.addPair("80 msec", 80);
            delayMap.addPair("100 msec", 100);
            
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_audiocodec_speex,
                                             container, false);
            
            exchangeSpeexCodec(rootView, false);
            
            return rootView;
        }
        
        void exchangeSpeexCodec(View rootView, boolean store) {
            
            Spinner sr = (Spinner)rootView.findViewById(R.id.speex_bandmodeSpin);
            SeekBar quality = (SeekBar)rootView.findViewById(R.id.speex_qualitySeekBar);
            Spinner delay = (Spinner)rootView.findViewById(R.id.speex_txmsecSpin);
            
            if(store) {
                speexcodec.nBandmode = srMap.getValue(sr.getSelectedItemPosition(),
                                                      SpeexConstants.DEFAULT_SPEEX_BANDMODE);
                speexcodec.nQuality = quality.getProgress() + SpeexConstants.SPEEX_QUALITY_MIN;
                speexcodec.nMSecPerPacket = delayMap.getValue(delay.getSelectedItemPosition(),
                                                              SpeexConstants.DEFAULT_SPEEX_DELAY);
            }
            else {
                sr.setAdapter(srMap);
                sr.setSelection(delayMap.getIndex(speexcodec.nBandmode, 1));
                quality.setMax(SpeexConstants.SPEEX_QUALITY_MAX - SpeexConstants.SPEEX_QUALITY_MIN);
                quality.setProgress(speexcodec.nQuality + SpeexConstants.SPEEX_QUALITY_MIN);
                delay.setAdapter(delayMap);
                delay.setSelection(delayMap.getIndex(speexcodec.nMSecPerPacket, 1));
            }
        }
    }
    
    public static class SpeexVBRFragment extends Fragment {

        SpeexVBRCodec speexvbrcodec;
        MapAdapter srMap;
        MapAdapter delayMap;

        public SpeexVBRFragment(SpeexVBRCodec speexvbrcodec) {
            this.speexvbrcodec = speexvbrcodec;
        }

        @Override
        public void onAttach(Activity activity) {
            
            srMap = new MapAdapter(activity);
            srMap.addPair("8 KHz", SpeexConstants.SPEEX_BANDMODE_NARROW);
            srMap.addPair("16 KHz", SpeexConstants.SPEEX_BANDMODE_WIDE);
            srMap.addPair("32 KHz", SpeexConstants.SPEEX_BANDMODE_UWIDE);
            
            delayMap = new MapAdapter(activity);
            delayMap.addPair("20 msec", 20);
            delayMap.addPair("40 msec", 40);
            delayMap.addPair("60 msec", 60);
            delayMap.addPair("80 msec", 80);
            delayMap.addPair("100 msec", 100);
            
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_audiocodec_speexvbr,
                                             container, false);
            
            exchangeSpeexVBRCodec(rootView, false);
            
            return rootView;
        }
        
        void exchangeSpeexVBRCodec(View rootView, boolean store) {
            
            Spinner sr = (Spinner) rootView.findViewById(R.id.speexvbr_bandmodeSpin);
            SeekBar quality = (SeekBar) rootView.findViewById(R.id.speexvbr_qualitySeekBar);
            // SeekBar bitrate = (SeekBar)rootView.findViewById(R.id.speexvbr_maxbrSeekBar);
            // final TextView bitrateText = (TextView)rootView.findViewById(R.id.speexvbr_brTextView);
            CheckBox dtx = (CheckBox) rootView.findViewById(R.id.speexvbr_dtxCheckBox);
            Spinner delay = (Spinner) rootView.findViewById(R.id.speexvbr_txmsecSpin);

            if(store) {
                speexvbrcodec.nBandmode = srMap.getValue(sr.getSelectedItemPosition(),
                                                         SpeexConstants.DEFAULT_SPEEX_BANDMODE);
                speexvbrcodec.nQuality = quality.getProgress() + SpeexConstants.SPEEX_QUALITY_MIN;
                speexvbrcodec.bDTX = dtx.isChecked();
                speexvbrcodec.nMSecPerPacket = delayMap.getValue(delay.getSelectedItemPosition(),
                                                                 SpeexConstants.DEFAULT_SPEEX_DELAY);
            }
            else {
                // sr.setOnItemSelectedListener(new OnItemSelectedListener() {
                // @Override
                // public void onItemSelected(AdapterView< ? > arg0,
                // View view, int position, long id) {
                // switch(srMap.getValue(position, SpeexConstants.DEFAULT_SPEEX_BANDMODE)) {
                // case SpeexConstants.SPEEX_BANDMODE_NARROW :
                // bitrate.setMax(SpeexConstants.SPEEX_NB_MAX_BITRATE -
                // SpeexConstants.SPEEX_NB_MIN_BITRATE);
                // break;
                // case SpeexConstants.SPEEX_BANDMODE_WIDE :
                // }
                // }
                // @Override
                // public void onNothingSelected(AdapterView< ? > arg0) {
                // }
                // });

                sr.setAdapter(srMap);
                sr.setSelection(delayMap.getIndex(speexvbrcodec.nBandmode, 1));
                quality.setMax(SpeexConstants.SPEEX_QUALITY_MAX - SpeexConstants.SPEEX_QUALITY_MIN);
                quality.setProgress(speexvbrcodec.nQuality - SpeexConstants.SPEEX_QUALITY_MIN);
                dtx.setChecked(speexvbrcodec.bDTX);
                delay.setAdapter(delayMap);
                delay.setSelection(delayMap.getIndex(speexvbrcodec.nMSecPerPacket, 1));
            }
        }
    }
   
    public static class NoAudioFragment extends Fragment {

        public NoAudioFragment() {
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_audiocodec_noaudio,
                                             container, false);
            return rootView;
        }
    }

}
