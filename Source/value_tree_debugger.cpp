//
// Created by Jim Credland on 21/06/2015.
//

#include "value_tree_debugger.h"

/** Display a tree. */
class ValueTreeDebuggerMain :
    public Component
{
public:
    class PropertyEditor :
        public PropertyPanel
    {
    public:
        PropertyEditor()
        {
            noEditValue = "not editable";
        }

        void setSource (ValueTree& newSource)
        {
            clear();

            tree = newSource;

            const int maxChars = 200;

            Array<PropertyComponent*> pc;

            for (int i = 0; i < tree.getNumProperties(); ++i)
            {
                const Identifier name = tree.getPropertyName (i).toString();
                Value v = tree.getPropertyAsValue (name, nullptr);
                TextPropertyComponent* tpc;

                if (v.getValue().isObject())
                {
                    tpc = new TextPropertyComponent (noEditValue, name.toString(), maxChars, false);
                    tpc->setEnabled (false);
                }
                else
                {
                    tpc = new TextPropertyComponent (v, name.toString(), maxChars, false);
                }

                pc.add (tpc);
            }

            addProperties (pc);
        }

    private:
        Value noEditValue;
        ValueTree tree;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyEditor);
    };

    class Item :
        public TreeViewItem,
        public ValueTree::Listener
    {
    public:
        Item (PropertyEditor* propertiesEditor, ValueTree tree)
            :
            propertiesEditor (propertiesEditor),
            t (tree)
        {
            t.addListener (this);
        }

        ~Item()
        {
            clearSubItems();
        }

        bool mightContainSubItems()
        {
            return t.getNumChildren() > 0;
        }

        void itemOpennessChanged (bool isNowOpen)
        {
            if (isNowOpen) updateSubItems();
        }

        void updateSubItems()
        {
            ScopedPointer<XmlElement> openness = getOpennessState();
            clearSubItems();
            int children = t.getNumChildren();

            for (int i = 0; i < children; ++i)
                addSubItem (new Item (propertiesEditor, t.getChild (i)));

            if (openness)
                restoreOpennessState (*openness);
        }

        void paintItem (Graphics& g, int w, int h)
        {

            Font font;
            Font smallFont (11.0);


            if (isSelected())
                g.fillAll (Colours::white);


            const float padding = 20.0f;

            String typeName = t.getType().toString();

            const float nameWidth = font.getStringWidthFloat (typeName);
            const float propertyX = padding + nameWidth;

            g.setColour (Colours::black);

            g.setFont (font);

            g.drawText (t.getType().toString(), 0, 0, w, h, Justification::left, false);
            g.setColour (Colours::blue);

            String propertySummary;

            for (int i = 0; i < t.getNumProperties(); ++i)
            {
                const Identifier name = t.getPropertyName (i).toString();
                String propertyValue = t.getProperty (name).toString();;
#ifdef JCF_SERIALIZER

                /* Code for handling valuetree serialization classes I use internally.
                 They allow ReferenceCountedObjects that are stored in the tree
                 to save, load and display debug information easily. You won't
                 need this but it makes my life easier to leave it here - Jim. */
                if (t[name].isObject())
                {
                    ReferenceCountedObject* p = t[name].getObject();

                    if (Serializable* s = dynamic_cast<Serializable*> (p))
                        propertyValue = "[[" + s->getDebugInformation() + "]]";
                }

#endif
                propertySummary += " " + name.toString() + "=" + propertyValue;
            }

            g.drawText (propertySummary, propertyX, 0, w - propertyX, h, Justification::left, true);
        }

        void itemSelectionChanged (bool isNowSelected)
        {
            if (isNowSelected)
            {
                t.removeListener (this);
                propertiesEditor->setSource (t);
                t.addListener (this);
            }
        }

        /* Enormous list of ValueTree::Listener options... */
        void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property)
        {
            if (t != treeWhosePropertyHasChanged) return;

            t.removeListener (this);
            //            if (isSelected())
            //                propertiesEditor->setSource(t);
            repaintItem();
            t.addListener (this);
        }

        void valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded)
        {
            if (parentTree == t)
                updateSubItems();

            treeHasChanged();
        }

        void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int)
        {
            if (parentTree == t)
                updateSubItems();

            treeHasChanged();
        }
        void valueTreeChildOrderChanged (ValueTree& parentTreeWhoseChildrenHaveMoved, int, int)
        {
            if (parentTreeWhoseChildrenHaveMoved == t)
                updateSubItems();

            treeHasChanged();
        }
        void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)
        {
            treeHasChanged();
        }
        void valueTreeRedirected (ValueTree& treeWhichHasBeenChanged)
        {
            if (treeWhichHasBeenChanged == t)
                updateSubItems();

            treeHasChanged();
        }

        /* Works only if the ValueTree isn't updated between calls to getUniqueName. */
        String getUniqueName() const
        {
            if (t.getParent() == ValueTree::invalid) return "1";

            return String (t.getParent().indexOf (t));
        }

    private:
        PropertyEditor* propertiesEditor;
        ValueTree t;
        Array<Identifier> currentProperties;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Item);
    };

    ValueTreeDebuggerMain() :
        layoutResizer (&layout, 1, false)
    {
        layout.setItemLayout (0, -0.1, -0.9, -0.6);
        layout.setItemLayout (1, 5, 5, 5);
        layout.setItemLayout (2, -0.1, -0.9, -0.4);

        setSize (1000, 700);
        addAndMakeVisible (treeView);
        addAndMakeVisible (propertyEditor);
        addAndMakeVisible (layoutResizer);
    }
    ~ValueTreeDebuggerMain()
    {
        treeView.setRootItem (nullptr);
    }

    void resized() override
    {
        Component* comps[] = { &treeView, &layoutResizer, &propertyEditor };
        layout.layOutComponents (comps, 3, 0, 0, getWidth(), getHeight(), true, true);
    }

    void setTree (ValueTree newTree)
    {
        if (newTree == ValueTree::invalid)
        {
            treeView.setRootItem (nullptr);
        }
        else if (tree != newTree)
        {
            tree = newTree;
            rootItem = new Item (&propertyEditor, tree);
            treeView.setRootItem (rootItem);
        }
    }

public:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreeDebuggerMain);

    ScopedPointer<Item> rootItem;
    ValueTree tree;
    TreeView treeView;
    PropertyEditor propertyEditor;
    StretchableLayoutManager layout;
    StretchableLayoutResizerBar layoutResizer;
};


ValueTreeDebugger::ValueTreeDebugger()
:
    DocumentWindow ("Value Tree Debugger",
                    Colours::lightgrey,
                    DocumentWindow::allButtons)
{
    construct();
}

ValueTreeDebugger::ValueTreeDebugger(ValueTree & tree)
:
    DocumentWindow ("Value Tree Debugger",
                    Colours::lightgrey,
                    DocumentWindow::allButtons)
{
    construct();
    setSource(tree);
}

void ValueTreeDebugger::construct()
{
    main = new ValueTreeDebuggerMain();
    setContentNonOwned (main, true);
    setResizable (true, false);
    setUsingNativeTitleBar (true);
    centreWithSize (getWidth(), getHeight());
    setVisible (true);
}

ValueTreeDebugger::~ValueTreeDebugger()
{
    main->setTree (ValueTree::invalid);
}

void ValueTreeDebugger::closeButtonPressed()
{
    setVisible (false);
}

void ValueTreeDebugger::setSource (ValueTree& v)
{
    main->setTree (v);
}
