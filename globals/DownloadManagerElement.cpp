#include "DownloadManagerElement.h"

// clang-format off
template<> Actor*         DownloadManagerElement::getElement() { return actor;   }
template<> Album*         DownloadManagerElement::getElement() { return album;   }
template<> Artist*        DownloadManagerElement::getElement() { return artist;  }
template<> Concert*       DownloadManagerElement::getElement() { return concert; }
template<> Movie*         DownloadManagerElement::getElement() { return movie;   }
template<> TvShow*        DownloadManagerElement::getElement() { return show;    }
template<> TvShowEpisode* DownloadManagerElement::getElement() { return episode; }
// clang-format on
