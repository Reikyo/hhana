import numpy as np
from . import log; log = log[__name__]


def tauid_sf_internal(
    event_number,
    tight_1,
    tight_2,
    sf_tight_1,
    sf_tight_2,
    sf_med_1,
    sf_med_2):

    # tight - medium
    if (tight_1 and not tight_2):
      return sf_tight_1 * sf_med_2
    
    # medium - tight
    if (not tight_1 and tight_2):
      return sf_med_1 * sf_tight_2;
    
    # tight - tight
    if (tight_1 and tight_2):
      if (event_number % 2 != 0):
          return sf_tight_1 * sf_med_2;
      else: 
          return sf_med_1 * sf_tight_2;
    
v_tauid_sf_internal = np.vectorize(tauid_sf_internal)


def tauid_sf(rec, systematic='NOMINAL'):
    
    log.debug('retrieve required fields for TAUID SF calculation')
    event_number = rec['event_number']
    tight_1 = rec['ditau_tau0_jet_bdt_tight']
    tight_2 = rec['ditau_tau1_jet_bdt_tight']
    sf_tight_1 = rec['ditau_tau0_sf_NOMINAL_TauEffSF_HLT_tau35_medium1_tracktwo_JETIDBDTTIGHT']
    sf_tight_2 = rec['ditau_tau1_sf_NOMINAL_TauEffSF_HLT_tau25_medium1_tracktwo_JETIDBDTTIGHT']
    sf_med_1 = rec['ditau_tau0_sf_NOMINAL_TauEffSF_HLT_tau35_medium1_tracktwo_JETIDBDTMEDIUM']
    sf_med_2 = rec['ditau_tau1_sf_NOMINAL_TauEffSF_HLT_tau25_medium1_tracktwo_JETIDBDTMEDIUM']
    log.debug('TAUID SF calculation')
    final_sf = v_tauid_sf_internal(
        event_number, tight_1, tight_2,
        sf_tight_1, sf_tight_2,
        sf_med_1, sf_med_2)
    log.debug('TAUID SF calculation done')
    return final_sf


def trigger_sf_internal(
    match_1,
    match_2,
    sf_1, 
    sf_2):

    return match_1 * sf_1 * match_2 * sf_2;

v_trigger_sf_internal = np.vectorize(trigger_sf_internal)

def trigger_sf(rec, systematic='NOMINAL'):

    log.debug('retrieve required fields for TRIGGER SF calculation')

    match_1 = rec['ditau_tau0_HLT_tau35_medium1_tracktwo']
    sf_1 = rec['ditau_tau0_sf_NOMINAL_TauEffSF_HLT_tau35_medium1_tracktwo_JETIDBDTMEDIUM']
    match_2 = rec['ditau_tau1_HLT_tau25_medium1_tracktwo']
    sf_2 = rec['ditau_tau1_sf_NOMINAL_TauEffSF_HLT_tau25_medium1_tracktwo_JETIDBDTMEDIUM']

    log.debug('TRIGGER SF calculation')
    final_sf = v_trigger_sf_internal(
        match_1, match_2, sf_1, sf_2)
    log.debug('TAUID SF calculation done')
    return final_sf
