#include "MainForm.hpp"
#include "DefinitionParser.hpp"

void MainForm::resize(const RectangleType& screenBounds)
{
    setBounds(screenBounds);

    RectangleType bounds;
    UInt16 index=getObjectIndex(definitionScrollBar);
    getObjectBounds(index, bounds);
    bounds.topLeft.x=screenBounds.extent.x-8;
    bounds.extent.y=screenBounds.extent.y-36;
    setObjectBounds(index, bounds);
    
    index=getObjectIndex(termLabel);
    getObjectBounds(index, bounds);
    bounds.topLeft.y=screenBounds.extent.y-14;
    setObjectBounds(index, bounds);

    index=getObjectIndex(termInputField);
    getObjectBounds(index, bounds);
    bounds.topLeft.y=screenBounds.extent.y-14;
    bounds.extent.x=screenBounds.extent.x-60;
    setObjectBounds(index, bounds);

    index=getObjectIndex(goButton);
    getObjectBounds(index, bounds);
    bounds.topLeft.x=screenBounds.extent.x-26;
    bounds.topLeft.y=screenBounds.extent.y-14;
    setObjectBounds(index, bounds);

    update();    
}

void MainForm::draw(UInt16 updateCode)
{
    iPediaForm::draw(updateCode);
    ArsLexis::Rectangle rect=bounds();
    WinPaintLine(rect.x(), rect.height()-18, rect.width(), rect.height()-18);

    rect.explode(2, 18, -12, -36);
    //! @todo Obtain RenderingPreferences from Application object and do the following right way...
    RenderingPreferences* prefs=0;
    definition_.render(rect, *prefs);
    UInt16 index=getObjectIndex(definitionScrollBar);
    ScrollBarType* scrollBar=static_cast<ScrollBarType*>(FrmGetObjectPtr(*this, index));
    SclSetScrollBar(scrollBar, definition_.firstShownLine(), 0, definition_.totalLinesCount()-definition_.shownLinesCount(), definition_.shownLinesCount());
}


Err MainForm::initialize()
{
    Err error=iPediaForm::initialize();
    if (!error)
    {
        ArsLexis::String text(            
"[[da:science-fiction]] [[de:science-fiction]] [[eo:sciencfikcio]] [[es:ciencia ficción]] [[fi:tieteiskirjallisuus]] [[fr:science-fiction]] [[he:&#1502;&#1491;&#1506; &#1489;&#1491;&#1497;&#1493;&#1504;&#1497;]][[ja:&#12469;&#12452;&#12456;&#12531;&#12473;&#12539;&#12501;&#12451;&#12463;&#12471;&#12519;&#12531;]] [[nl:science fiction]] [[pl:science fiction]] [[sv:science fiction]][[ru:&#1053;&#1072;&#1091;&#1095;&#1085;&#1072;&#1103; &#1092;&#1072;&#1085;&#1090;&#1072;&#1089;&#1090;&#1080;&#1082;&#1072;]]\n"
"\n"
"'''science fiction''' is a form of [[fiction]] which deals principally with the impact of actual or imagined science (and/or technology) upon society or individuals. \n"
"\n"
"==scope==\n"
"sometimes the characters involved are not even human, but are imagined aliens or other products of earth evolution. the term is more generally used to refer to any literary fantasy that includes a [[science|scientific]] factor as an essential orienting component, and even more generally used to refer to any [[fantasy]] at all. such literature may consist of a careful and informed extrapolation of scientific facts and principles, or it may range into far-fetched areas flatly contradictory of such facts and principles. in either case, plausibility based on science is a requisite, so that such precursors of the genre as [[mary wollstonecraft shelley]]'s gothic novel ''[[frankenstein]], or the modern prometheus'' ([[1818]]) and [[robert louis stevenson]]'s ''strange case of dr. jekyll and mr. hyde'' ([[1886]]) are plainly science fiction, whereas [[bram stoker]]'s ''[[dracula]]'' ([[1897]]), based purely on the [[supernatural]], is not.\n"
"\n"
"==types of science fiction==\n"
"\n"
"===hard science fiction===\n"
"''main article'': [[hard science fiction]]\n"
"\n"
"hard science fiction, or hard sf, is a subgenre of science fiction characterized by an interest in [[science|scientific]] detail or accuracy.  hard sf stories focus on the natural sciences and technological developments.  some authors scrupulously eschew such implausibilities as faster-than-light travel, while others accept such plot devices but nonetheless show a concern with a realistic depiction of the worlds that such a technology might make accessible. character development is sometimes secondary to explorations of [[astronomy|astronomical]] or [[physics|physical]] phenomena, but other times authors make the human condition forefront in the story. however a common theme of hard sf has the resolution of the plot often hinging upon a technological point.  writers attempt to have their stories consistent with known science at the time of publication. \n"
"\n"
"===soft science fiction===\n"
"''main article'': [[soft science fiction]]\n"
"\n"
"[[soft science fiction]] is [[science fiction]] whose plots and themes tend to focus on [[philosophy]], [[psychology]], [[politics]] and [[sociology]] while de-emphasizing the details of technological hardware and physical laws.  it is so-called 'soft' science fiction, because these subjects are grouped together as the soft sciences or humanities. for instance, in ''[[dune (novel)|dune]]'', [[frank herbert]] uses the plot device of a universe which has rejected conscious machines and has reverted to a feudal society. consequently herbert uses the dune saga to comment about the human condition and make direct parallels to current socio-political realities. soft science fiction may explore the reactions of societies or individuals to problems posed by natural phenomena or technological developments, but the technology will be a means to an end, not an end itself.\n"
"\n"
"===other types===\n"
"there are, of course, many borderline cases of works using outer-space settings and futuristic-looking technology as little more than window-dressing for tales of adventure, romance, and other typical dramatic themes; examples include [[star wars]] (which is considered by some diehards to be not science fiction but [[fantasy]]) and many hollywood [[space opera]]s. some fans of [[hard science fiction]] would regard such films as fantasy, whereas the general public would probably place them squarely in the science fiction category. it has been suggested as a method of resolving this confusion that sf come to stand for [[speculative fiction]] and thus encompass [[fantasy]], [[horror]], and [[sci-fi]] genres.\n"
"\n"
"==history of science fiction==\n"
"\n"
"===forerunners of science fiction===\n"
"science fiction was made possible only by the rise of modern science itself, notably the revolutions in astronomy and physics.  aside from the age-old genre of fantasy literature, which does not qualify, there were notable precursors: \n"
"imaginary voyages to the moon in the 17th century, first shown in [[johannes kepler]]'s ''[[somnium]]'' (the dream, [[1634]]), then in [[cyrano de bergerac]]'s ''[[comical history of the states and empires of the moon]]''  ([[1656]]), space travel in [[voltaire]]'s ''[[micromégas]]'' ([[1752]]), alien cultures in [[jonathan swift]]'s ''[[gulliver's travels]]'' ([[1726]]), and science fiction elements in the 19th-century stories of [[edgar allan poe]], [[nathaniel hawthorne]], and [[fitz-james o'brien]]. in [[romantic poetry]], too, the writers' imaginations leapt to visions of other worlds and distant futures as in [[alfred lord tennyson]]'s '[[locksley hall]]'.\n"
"\n"
"===early science fiction===\n"
"\n"
"the european brand of science fiction proper began, however, toward the end of the 19th century with the scientific romances of [[jules verne]], whose science was rather on the level of invention, as well as the science-oriented novels of social criticism by [[h.g. wells]].  although better known for other works, [[sir arthur conan doyle]] also wrote early science fiction.\n"
"\n"
"the development of american science fiction as a self-conscious [[genre]] dates (in part) from [[1926]], when [[hugo gernsback]] founded ''[[amazing stories]]'' magazine, which was devoted exclusively to science fiction stories. since he is notable for having chosen the variant term ''scientifiction'' to describe this incipient genre, the stage in the genre's development, his name and the term \"scientifiction\" are often thought to be inextricably linked. published in this and other [[pulp magazine|pulp magazines]] with great and growing success, such scientifiction stories were not viewed as serious literature but as sensationalism.\n"
"\n"
"===the '''golden''' age===\n"
"''main article'' : [[astounding magazine]]\n"
"\n"
"with the advent in [[1937]] of a demanding editor, [[john w. campbell|john w. campbell, jr.]], of ''[[analog science fiction|astounding science fiction]]'' (founded in [[1930]]), and with the publication of stories and novels by such writers as [[isaac asimov]], [[arthur c. clarke]], and [[robert heinlein]], science fiction emerged as a mode of serious fiction.  \n"
"ventures into the genre by writers who were not devoted exclusively to science fiction, such as [[karel capek]], [[aldous huxley]], [[c. s. lewis]] and, later, [[ray bradbury]] and [[kurt vonnegut]] jr., also added respectability. magazine covers of bug-eyed monsters and scantily-clad women preserved the sensational image for many, however.\n"
"\n"
"===the post-war era===\n"
"a great boom in the popularity of science fiction followed [[world war ii]].  \n"
"some science fiction works became paperback best-sellers. \n"
"\n"
"===the modern era===\n"
"the modern era began in the  mid 1960's with the popularisation of the genre of [[soft science fiction]]. in literary terms it dates roughly from the publication of [[frank herbert]]'s ''[[dune (novel)|dune]]'' in 1965, a dense, complex, and detailed work of fiction featuring political intrigue in a future galaxy, strange and mystical religious beliefs, and the eco-system of the desert planet [[arrakis]]. while in 1966 [[gene roddenberry]]'s ''[[star trek]]'' brought such science fiction to a mass television audience. the original ''star trek'' seems out of date today, but at the time it was at the forefront of [[liberalism]]. it preached the universality and equality of humanity. it had an attractive black officer, the first black-white kiss, a russian officer at the height of the cold war, an asian officer, and even an alien officer.\n"
"\n"
"the field saw an increase in:\n"
"*the number of writers and readers \n"
"*the breadth of subject matter \n"
"*the depth of treatment \n"
"*the sophistication of language and technique \n"
"*the political and literary consciousness of the writing. \n"
"\n"
"also, technological fixes to a problem became a far rarer plot device.\n"
"\n"
"a second generation of original and popular science fiction films begin to appear, among the most significant of which were ''[[2001: a space odyssey]]'' ([[1968]]), ''[[thx 1138]]'' ([[1969]]) ''[[close encounters of the third kind]]'', ([[1977]]), and ''[[star wars]]'', ([[1977]]). (see the [[list of science fiction films]] article for a more detailed list of notable science fiction films).\n"
"\n"
"the success of ''[[star wars]]'' was especially influential since it caused an explosive increase of interest in science fiction for several years after its release (though this has since abated, somewhat). science fiction literature strongly benefitted from this heightened interest and science fiction or fantasy titles frequently filled the bestseller lists well into the [[1980s]].  eventually, cultural interest in science fiction declined somewhat with cultural fatigue, flooded markets, and competition from other entertainment venues being a few of the reasons for this. also, science fictional or fantasy \"elements\" began to be usurped by traditional authors and other types of media, though they were not significant enough to be classified as purely science fiction or fantasy. today, ''pure'' science fiction or fantasy books only occasionally make the bestseller lists, although, in overall numbers there are more science fiction or fantasy books published now than in the past.\n"
"\n"
"the influence of fantasy on the genre resulted in what is now called science fantasy. contributions of these works to the literature of the fantastic include an awareness of irrationality and the inexplicable, the transformative force of language, and the power of myth to organize experience. ''[[star wars]]'' ([[1977]]) is the most powerful example of this trend.\n"
"\n"
"the increasing intellectual sophistication of the genre and the emphasis on wider societal and psychological issues significantly broadened the appeal of science fiction to the reading public. science fiction became international, extending into the then [[soviet union]] and other eastern european nations, where it was frequently used as a vehicle for political commentary that could not be safely published in other forms. the polish author [[stanislaw lem]] is one of the non-english science fiction writers who has become widely known outside his native country. serious criticism of the genre is now common, and science fiction is studied in colleges and universities, both as literature and in how it relates to science and society.\n"
"\n"
"the principal science fiction awards are the [[hugo_award|hugo]] and [[nebula_award|nebula]].\n"
"\n"
"science fiction has also been popular in [[radio]], [[comic books]], [[television]], and [[movies]]; it is notable that about three-quarters of the [[top twenty highest grossing films]] (source: imdb june 2002) are based around science-fiction or fantasy themes.\n"
"\n"
"== fandom ==\n"
"one of the unique features of the science fiction genre is its strong fan community, of which many authors are a firm part. many people interested in science-fiction wish to interact with others who share the same interests; over time an entire culture of [[science fiction fandom]] has evolved. local fan groups exist in most of the english-speaking world, as well as in japan, europe, and elsewhere; these groups often publish their own works. also, fans were the originator of [[science fiction convention]]s, which gave them a way of getting together to discuss their mutual interest.  the original and largest convention is the [[worldcon]].\n"
"\n"
"many  [[fanzine]]s (\"fan magazines\") (and a few professional ones) exist that are dedicated solely to informing the science fiction fan on all aspects of the genre.  \n"
"the premiere awards of science fiction, the [[hugo award]]s, are awarded by members of the annual [[worldcon]], which is almost entirely volunteer-run by fans.\n"
"\n"
"science fiction fandom often overlaps with other similar interests, such as fantasy, [[role playing games]] and the [[society for creative anachronism]].\n"
"\n"
"== genres and subcategories ==\n"
"*[[hard science fiction]]\n"
"*[[soft science fiction]]\n"
"*[[space opera]]\n"
"*[[military science fiction]]\n"
"*[[science fantasy]]\n"
"*[[cyberpunk]]\n"
"*[[apocalyptic and post-apocalyptic science fiction]]\n"
"*[[postcyberpunk]]\n"
"*[[social fiction]]\n"
"*[[clerical fiction]]\n"
"*[[political fiction]]\n"
"*[[new wave (science fiction)]]\n"
"*[[alternate history]]\n"
"*[[utopian and dystopian fiction]]\n"
"*[[comic science fiction]]\n"
"*[[science fiction sitcom]]\n"
"*[[science fiction erotica]]\n"
"*[[lesbian science fiction]]\n"
"*[[space-rock]]\n"
"*[[xenofiction]]\n"
"\n"
"== related topics ==\n"
"*[[list of science fiction authors]]\n"
"*[[women in science fiction]]\n"
"*[[science fiction convention]]s\n"
"*[[science fiction film]]\n"
"*[[science fiction television]]\n"
"*[[list of science fiction visual artists]]\n"
"*[[science fiction fandom]]\n"
"*[[list of science fiction awards]]\n"
"*[[fantasy]]\n"
"*[[science fiction and fantasy writers of america]] \n"
"*[[science fiction themes]]\n"
"*[[list of science fiction novels]]\n"
"*[[list of science fiction short stories]]\n"
"*[[list of science fiction television]]\n"
"*[[sex in science fiction]]\n"
"*[[computers in fiction]]\n"
"*[[fictional technology]]\n"
"*[[planets in science fiction]]\n"
"\n"
"== external links ==\n"
"* [http://isfdb.tamu.edu/ internet speculative fiction database]\n"
"* ''[[the encyclopedia of science fiction]]'' by [[john clute]] (ed.) and [[peter nicholls]] (ed.) isbn 031213486x\n"
"* [http://www.nvcc.edu/home/ataormina/scifi/default.htm excellent science fiction guide]\n"
"* [http://wondersmith.com/scifi/index.htm 'free sci-fi classics' a website providing archives of full text, non-copyright science fiction by voltaire, mary shelley, mark twain, jules verne, h.g. wells, rudyard kipling and others]\n"
    );        
        DefinitionParser parser;
        parser.parse(text, definition_);                
    }
    return error;
}

inline void MainForm::handleScrollRepeat(const sclRepeat& data)
{
    definition_.scroll(data.newValue-data.value);
}

void MainForm::handlePenUp(const EventType& event)
{
    PointType point;
    point.x=event.screenX;
    point.y=event.screenY;
    if (definition_.bounds() && point)
        definition_.hitTest(point); 
}


Boolean MainForm::handleEvent(EventType& event)
{
    Boolean handled=false;
    switch (event.eType)
    {
        case penUpEvent:
            handlePenUp(event);
            break;
            
        case sclRepeatEvent:
            handleScrollRepeat(event.data.sclRepeat);
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}