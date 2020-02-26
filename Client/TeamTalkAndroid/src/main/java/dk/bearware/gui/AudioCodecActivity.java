/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

package dk.bearware.gui;

import java.util.Locale;

import dk.bearware.AudioCodec;
import dk.bearware.Channel;
import dk.bearware.Codec;
import dk.bearware.OpusCodec;
import dk.bearware.OpusConstants;
import dk.bearware.SpeexCodec;
import dk.bearware.SpeexConstants;
import dk.bearware.SpeexVBRCodec;
import dk.bearware.backend.TeamTalkConstants;
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
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audiocodec);

        audiocodec = Utils.getAudioCodec(this.getIntent());

        int tab_index = 0;
        switch(audiocodec.nCodec) {
            case Codec.OPUS_CODEC :
                tab_index = TAB_OPUS;
                break;
            case Codec.SPEEX_CODEC :
                tab_index = TAB_SPEEX;
                break;
            case Codec.SPEEX_VBR_CODEC :
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
                        audiocodec.opus = opusfrag.exchangeOpusCodec(opusfrag.getView(), true);
                        audiocodec.nCodec = Codec.OPUS_CODEC;
                        break;
                    }
                    case TAB_SPEEX : {
                        SpeexFragment spxfrag = (SpeexFragment)frag;
                        audiocodec.speex = spxfrag.exchangeSpeexCodec(spxfrag.getView(), true);
                        audiocodec.nCodec = Codec.SPEEX_CODEC;
                        break;
                    }
                    case TAB_SPEEXVBR : {
                        SpeexVBRFragment spxfrag = (SpeexVBRFragment)frag;
                        audiocodec.speex_vbr = spxfrag.exchangeSpeexVBRCodec(spxfrag.getView(), true);
                        audiocodec.nCodec = Codec.SPEEX_VBR_CODEC;
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
            default :
                return super.onOptionsItemSelected(item);
        }
        
        return true;
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
                        opusfrag = new OPUSFragment();
                    return opusfrag;
                case TAB_SPEEX :
                    if(speexfrag == null)
                        speexfrag = new SpeexFragment();
                    return speexfrag;
                case TAB_SPEEXVBR :
                    if(speexvbrfrag == null)
                        speexvbrfrag = new SpeexVBRFragment();
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
        MapAdapter fsMap;

        public OPUSFragment() {
        }

        @Override
        public void onAttach(Activity activity) {
            opuscodec = ((AudioCodecActivity)activity).audiocodec.opus;
            appMap = new MapAdapter(activity, R.layout.item_spinner, R.id.spinTextView);
            srMap = new MapAdapter(activity, R.layout.item_spinner, R.id.spinTextView);
            audMap = new MapAdapter(activity, R.layout.item_spinner, R.id.spinTextView);
            fsMap = new MapAdapter(activity, R.layout.item_spinner, R.id.spinTextView);

            appMap.addPair("VoIP", OpusConstants.OPUS_APPLICATION_VOIP);
            appMap.addPair("Music", OpusConstants.OPUS_APPLICATION_AUDIO);

            srMap.addPair("8 KHz", 8000);
            srMap.addPair("12 KHz", 12000);
            srMap.addPair("16 KHz", 16000);
            srMap.addPair("24 KHz", 24000);
            srMap.addPair("48 KHz", 48000);

            audMap.addPair("Mono", 1);
            audMap.addPair("Stereo", 2);

            fsMap.addPair("Default", TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC);
            fsMap.addPair("2.5 msec", OpusConstants.OPUS_MIN_FRAMESIZE);
            fsMap.addPair("5 msec", 5);
            fsMap.addPair("10 msec", 10);
            fsMap.addPair("20 msec", 20);
            fsMap.addPair("40 msec", 40);
            fsMap.addPair("60 msec", OpusConstants.OPUS_MAX_FRAMESIZE);
            fsMap.addPair("80 msec", 80);
            fsMap.addPair("100 msec", 100);
            fsMap.addPair("120 msec", OpusConstants.OPUS_REALMAX_FRAMESIZE);

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
        
        OpusCodec exchangeOpusCodec(View rootView, boolean store) {

            Spinner app = rootView.findViewById(R.id.opus_appSpin);
            Spinner sr = rootView.findViewById(R.id.opus_samplerateSpin);
            Spinner audchan = rootView.findViewById(R.id.opus_audchanSpin);
            CheckBox dtx = rootView.findViewById(R.id.opus_dtxCheckBox);
            CheckBox vbr = rootView.findViewById(R.id.opus_vbrCheckBox);
            SeekBar bitrate = rootView.findViewById(R.id.opus_bitrateSeekBar);
            final TextView bitrateText = rootView.findViewById(R.id.opus_brTextView);
            Spinner framesize = rootView.findViewById(R.id.opus_fsmsecSpin);
            SeekBar txinterval = rootView.findViewById(R.id.opus_txintervalSeekBar);
            final TextView txintervalText = rootView.findViewById(R.id.opus_txintervalTextView);

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
                opuscodec.bVBR = vbr.isChecked();
                opuscodec.bVBRConstraint = OpusConstants.DEFAULT_OPUS_VBRCONSTRAINT;
                opuscodec.nBitRate = bitrate.getProgress() * 1000 + OpusConstants.OPUS_MIN_BITRATE;
                opuscodec.nTxIntervalMSec = txinterval.getProgress() + TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
                opuscodec.nFrameSizeMSec = fsMap.getValue(framesize.getSelectedItemPosition(),
                                                          TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC);
            }
            else {
                app.setAdapter(appMap);
                app.setSelection(appMap.getIndex(opuscodec.nApplication, 0));

                sr.setAdapter(srMap);
                sr.setSelection(srMap.getIndex(opuscodec.nSampleRate, 0));
                
                dtx.setChecked(opuscodec.bDTX);
                
                audchan.setAdapter(audMap);
                audchan.setSelection(audMap.getIndex(opuscodec.nChannels, 0));

                int maxbr = OpusConstants.OPUS_MAX_BITRATE - OpusConstants.OPUS_MIN_BITRATE;
                bitrate.setMax(maxbr / 1000);

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

                bitrate.setProgress((opuscodec.nBitRate / 1000) - (OpusConstants.OPUS_MIN_BITRATE / 1000));

                int maxtxinterval = TeamTalkConstants.OPUS_MAX_TXINTERVALMSEC - TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
                txinterval.setMax(maxtxinterval);
                txinterval.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        int interval = progress + TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
                        txintervalText.setText(interval + " msec");

                        int selFramesize = fsMap.getValue(framesize.getSelectedItemPosition(), TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC);
                        if (interval > OpusConstants.OPUS_REALMAX_FRAMESIZE && selFramesize == TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC)
                            framesize.setSelection(fsMap.getIndex(OpusConstants.OPUS_REALMAX_FRAMESIZE, 0));
                        else if (selFramesize > interval)
                            framesize.setSelection(fsMap.getIndex(TeamTalkConstants.OPUS_DEFAULT_FRAMESIZEMSEC, 0));
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                    }
                });

                txinterval.setProgress(opuscodec.nTxIntervalMSec - TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC);

                framesize.setAdapter(fsMap);
                framesize.setSelection(fsMap.getIndex(opuscodec.nFrameSizeMSec, 0));
            }
            return opuscodec;
        }
    }
    
    public static class SpeexFragment extends Fragment {

        SpeexCodec speexcodec;

        MapAdapter srMap;

        public SpeexFragment() {
        }

        @Override
        public void onAttach(Activity activity) {
            speexcodec = ((AudioCodecActivity)activity).audiocodec.speex;
            srMap = new MapAdapter(activity, R.layout.item_spinner, R.id.spinTextView);
            srMap.addPair("8 KHz", SpeexConstants.SPEEX_BANDMODE_NARROW);
            srMap.addPair("16 KHz", SpeexConstants.SPEEX_BANDMODE_WIDE);
            srMap.addPair("32 KHz", SpeexConstants.SPEEX_BANDMODE_UWIDE);
            
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
        
        SpeexCodec exchangeSpeexCodec(View rootView, boolean store) {
            
            Spinner sr = rootView.findViewById(R.id.speex_bandmodeSpin);
            SeekBar quality = rootView.findViewById(R.id.speex_qualitySeekBar);
            SeekBar txinterval = rootView.findViewById(R.id.speex_txintervalSeekBar);
            final TextView txintervalText = rootView.findViewById(R.id.speex_txintervalTextView);
            
            if(store) {
                speexcodec.nBandmode = srMap.getValue(sr.getSelectedItemPosition(),
                                                      SpeexConstants.DEFAULT_SPEEX_BANDMODE);
                speexcodec.nQuality = quality.getProgress() + SpeexConstants.SPEEX_QUALITY_MIN;
                speexcodec.nTxIntervalMSec = txinterval.getProgress() + TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC;
            }
            else {
                sr.setAdapter(srMap);
                sr.setSelection(srMap.getIndex(speexcodec.nBandmode, 1));
                quality.setMax(SpeexConstants.SPEEX_QUALITY_MAX - SpeexConstants.SPEEX_QUALITY_MIN);
                quality.setProgress(speexcodec.nQuality + SpeexConstants.SPEEX_QUALITY_MIN);

                int maxtxinterval = TeamTalkConstants.SPEEX_MAX_TXINTERVALMSEC - TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
                txinterval.setMax(maxtxinterval);
                txinterval.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        int interval = progress + TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC;
                        txintervalText.setText(interval + " msec");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                    }
                });

                txinterval.setProgress(speexcodec.nTxIntervalMSec - TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC);

            }
            return speexcodec;
        }
    }
    
    public static class SpeexVBRFragment extends Fragment {

        SpeexVBRCodec speexvbrcodec;
        MapAdapter srMap;

        public SpeexVBRFragment() {
        }

        @Override
        public void onAttach(Activity activity) {
            speexvbrcodec = ((AudioCodecActivity)activity).audiocodec.speex_vbr;
            srMap = new MapAdapter(activity, R.layout.item_spinner, R.id.spinTextView);
            srMap.addPair("8 KHz", SpeexConstants.SPEEX_BANDMODE_NARROW);
            srMap.addPair("16 KHz", SpeexConstants.SPEEX_BANDMODE_WIDE);
            srMap.addPair("32 KHz", SpeexConstants.SPEEX_BANDMODE_UWIDE);
            
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
        
        SpeexVBRCodec exchangeSpeexVBRCodec(View rootView, boolean store) {
            
            Spinner sr = rootView.findViewById(R.id.speexvbr_bandmodeSpin);
            SeekBar quality = rootView.findViewById(R.id.speexvbr_qualitySeekBar);
            // SeekBar bitrate = rootView.findViewById(R.id.speexvbr_maxbrSeekBar);
            // final TextView bitrateText = rootView.findViewById(R.id.speexvbr_brTextView);
            CheckBox dtx = rootView.findViewById(R.id.speexvbr_dtxCheckBox);
            SeekBar txinterval = rootView.findViewById(R.id.speexvbr_txintervalSeekBar);
            final TextView txintervalText = rootView.findViewById(R.id.speexvbr_txintervalTextView);

            if(store) {
                speexvbrcodec.nBandmode = srMap.getValue(sr.getSelectedItemPosition(),
                                                         SpeexConstants.DEFAULT_SPEEX_BANDMODE);
                speexvbrcodec.nQuality = quality.getProgress() + SpeexConstants.SPEEX_QUALITY_MIN;
                speexvbrcodec.bDTX = dtx.isChecked();
                speexvbrcodec.nTxIntervalMSec = txinterval.getProgress() + TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC;
            }
            else {
                sr.setAdapter(srMap);
                sr.setSelection(srMap.getIndex(speexvbrcodec.nBandmode, 1));
                quality.setMax(SpeexConstants.SPEEX_QUALITY_MAX - SpeexConstants.SPEEX_QUALITY_MIN);
                quality.setProgress(speexvbrcodec.nQuality - SpeexConstants.SPEEX_QUALITY_MIN);
                dtx.setChecked(speexvbrcodec.bDTX);

                int maxtxinterval = TeamTalkConstants.SPEEX_MAX_TXINTERVALMSEC - TeamTalkConstants.OPUS_MIN_TXINTERVALMSEC;
                txinterval.setMax(maxtxinterval);
                txinterval.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        int interval = progress + TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC;
                        txintervalText.setText(interval + " msec");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                    }
                });

                txinterval.setProgress(speexvbrcodec.nTxIntervalMSec - TeamTalkConstants.SPEEX_MIN_TXINTERVALMSEC);
            }

            return speexvbrcodec;
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
